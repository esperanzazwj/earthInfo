#include "android.hpp"

#include "../util/compat/os.macro.hpp"
#ifdef SS_UTIL_ANDROID_COMPATIBLE
#include "../util/compat/os.undef.hpp"

#include "../util/constraint.hpp"
#include "../lib/native.hpp"
#include <list>
#include <vector>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <gestureDetector.h>
#include <android_native_app_glue.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <time.h>
#include <unistd.h>

#include "../util/assert.hpp"

// gestureDetector includes JNIHelper, which defines these
#undef LOGI
#undef LOGE

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "ssEngine GLFW Polyfill", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "ssEngine GLFW Polyfill", __VA_ARGS__)

namespace
{
    ::android_app* app;

    inline namespace polyfill
    {
        inline namespace glfw
        {
            bool glfw_initialized;
            std::list<GLFWwindow> glfw_windows;
            ::GLFWwindow* ctx;
            ::timespec init_time;
            static constexpr auto display_scaling = 2.0f;   // HiDPI support

            template <class Fn>
            Fn glfw_stub_set_callback(Fn newfn)
            {
                static Fn fn = nullptr;
                auto oldfn = fn;
                fn = newfn;
                return oldfn;
            }

            template <class ...Params>
            struct Event_Slot
            {
                using listener_type = void (Params...);

                auto listen(listener_type* listener) -> listener_type*
                {
                    auto old = this->listener;
                    this->listener = listener;
                    return old;
                }

                void emit(Params... xs)
                {
                    if (listener) listener(xs...);
                }

            private:
                listener_type* listener{};
            };

            struct Pinch_Zoom_Recognizer
            {
                // Feed in motion event.
                // Returns 0.0f if not pinched.
                // Returns x > 1.0f if "zoom in" gesture detected.
                // Returns 0.0f < x < 1.0f if "zoom out" gesture detected.
                auto feed(::AInputEvent const* ev) -> float
                {
                    auto state = pinch.Detect(ev);

                    if (state & ndk_helper::GESTURE_STATE_START) {
                        ndk_helper::Vec2 v0;
                        ndk_helper::Vec2 v1;
                        pinch.GetPointers(v0, v1);

                        distance = (v0 - v1).Length();
                        return 0.0f;
                    }

                    if (state & ndk_helper::GESTURE_STATE_MOVE) {
                        ndk_helper::Vec2 v0;
                        ndk_helper::Vec2 v1;
                        pinch.GetPointers(v0, v1);

                        auto old_distance = distance < 1e-3f ? 1e-3f : distance;
                        distance = (v0 - v1).Length();
                        return distance / old_distance;
                    }

                    return 0.0f;
                }

            private:
                ndk_helper::PinchDetector pinch{};
                float distance{};
            };

            struct Drag_Move_Recognizer
            {
                // Feed in motion event.
                // Returns 0 if not dragged.
                // Returns 1 if starting simply dragged.
                // Returns 2 if starting double tapped then dragged.
                // Returns 3 if dragging.
                // Returns -1 if simply dragging stopped.
                // Returns -2 if double dragging stopped.
                auto feed(::AInputEvent const* ev) -> int
                {
                    auto dbtap_state = double_tap.Detect(ev);
                    auto drag_state = drag.Detect(ev);

                    double_tapped |= dbtap_state & ndk_helper::GESTURE_STATE_ACTION;

                    if (drag_state & (ndk_helper::GESTURE_STATE_START | ndk_helper::GESTURE_STATE_END)) {
                        auto result = -drag_start_taps;
                        drag_start_taps = 0;
                        return result;
                    }

                    if (drag_state & ndk_helper::GESTURE_STATE_MOVE) {
                        if (drag_start_taps == 0) {
                            ndk_helper::Vec2 pos;
                            drag.GetPointer(pos);
                            pos.Value(x_, y_);
                            drag_start_taps = double_tapped ? 2 : 1;
                            double_tapped = false;
                            return drag_start_taps;
                        }

                        auto old_x = x_;
                        auto old_y = y_;

                        ndk_helper::Vec2 pos;
                        drag.GetPointer(pos);
                        pos.Value(x_, y_);

                        if (x_ == old_x && y_ == old_y)
                            return 0;
                        return 3;
                    }

                    return 0;
                }

                auto x() const -> float { return x_ / display_scaling; }
                auto y() const -> float { return y_ / display_scaling; }

            private:
                ndk_helper::DragDetector drag{};
                ndk_helper::DoubletapDetector double_tap{};
                float x_{};
                float y_{};
                int drag_start_taps{};
                bool double_tapped{};
            };
        }

        inline namespace stdio_logger
        {
            auto redirect_stdio(int fd, std::string description, int log_prio)
            {
                int pipes[2];
                ::pipe(pipes);
                ::dup2(pipes[1], fd);
                ::close(pipes[1]);

                std::thread{[pipe=pipes[0], desc=std::move(description), log_prio] {
                    auto input = ::fdopen(pipe, "r");
                    char buf[512];
                    auto tag = "ssEngine " + desc;
                    while (true) {
                        ::fgets(buf, sizeof(buf), input);
                        __android_log_write(log_prio, tag.data(), buf);
                    }
                }}.detach();
            }
        }
    }
}

namespace ss
{
    inline namespace polyfill
    {
        namespace android
        {
            auto Asset::slurp(util::As_CStr path) -> std::string
            {
                auto asset = ::AAssetManager_open(app->activity->assetManager, path, ::AASSET_MODE_BUFFER);
                if (asset == nullptr)
                    throw std::runtime_error{"Failed to open asset " + path.clone_as_str() + " for slurping"};

                auto buf = static_cast<char const*>(::AAsset_getBuffer(asset));
                if (buf == nullptr)
                    throw std::runtime_error{"Failed to slurp asset " + path.clone_as_str()};

                auto len = static_cast<size_t>(::AAsset_getLength64(asset));
                std::string result{buf, buf+len};

                ::AAsset_close(asset);
                return result;
            }

            auto Asset::readable(util::As_CStr path) -> bool
            {
                auto asset = ::AAssetManager_open(app->activity->assetManager, path, ::AASSET_MODE_UNKNOWN);
                if (asset == nullptr) {
                    return false;
                } else {
                    ::AAsset_close(asset);
                    return true;
                }
            }

            struct Asset_Internal_State: util::Non_Transferable
            {
                ::AAsset* asset;
                bool eof = false;

                Asset_Internal_State(util::As_CStr path)
                    : asset{::AAssetManager_open(app->activity->assetManager, path, ::AASSET_MODE_RANDOM)}
                {
                    if (asset == nullptr)
                        throw std::runtime_error{"Failed to open asset " + path.clone_as_str()};
                }

                ~Asset_Internal_State()
                {
                    if (asset) ::AAsset_close(asset);
                }
            };

            Asset::Asset(util::As_CStr path)
                : state{std::make_unique<Asset_Internal_State>(path)}
            {
            }

            auto Asset::read(void* buf, int size) -> int
            {
                auto n = ::AAsset_read(state->asset, buf, size);
                if (n < 0) throw std::runtime_error{"Failed reading file"};
                if (n == 0) state->eof = true;
                return n;
            }

            auto Asset::size() -> int
            {
                return ::AAsset_getLength(state->asset);
            }

            auto Asset::tell() -> int
            {
                return size() - ::AAsset_getRemainingLength(state->asset);
            }

            constexpr auto seek_error = ::off_t(-1);

            void Asset::seek(int offset)
            {
                state->eof = false;
                if (::AAsset_seek(state->asset, offset, SEEK_CUR) == seek_error)
                    throw std::runtime_error{"Failed to seek"};
            }

            void Asset::seek_front()
            {
                state->eof = false;
                if (::AAsset_seek(state->asset, 0, SEEK_SET) == seek_error)
                    throw std::runtime_error{"Failed to seek"};
            }

            void Asset::seek_back()
            {
                state->eof = false;
                if (::AAsset_seek(state->asset, 0, SEEK_END) == seek_error)
                    throw std::runtime_error{"Failed to seek"};
            }

            auto Asset::eof() const -> bool
            {
                return state->eof;
            }

            Asset::~Asset() = default;
        }
    }
}

extern "C"
{
    int main();
    int32_t glfw_handle_android_input(::android_app* app, ::AInputEvent* ev);
    void android_main(::android_app* app)
    {
        ::app = app;

        redirect_stdio(STDOUT_FILENO, "output", ANDROID_LOG_INFO);
        redirect_stdio(STDERR_FILENO, "debug", ANDROID_LOG_DEBUG);

        app->onAppCmd = [] (auto app, auto cmd) {
            switch (cmd) {
                case APP_CMD_INIT_WINDOW:
                    if (app->window) {
                        LOGI("Window is ready.");
                        app->onAppCmd = nullptr;
                        std::exit(main());
                    }
                    break;
                default: break;
            }
        };

        while (app->destroyRequested == 0) {
            ::android_poll_source* source{};
            ALooper_pollAll(-1, nullptr, nullptr, (void**)&source);
            if (source) source->process(app, source);
        }

        std::exit(0);
    }

    struct GLFWwindow final: ss::util::Non_Transferable
    {
        EGLDisplay display = EGL_NO_DISPLAY;
        EGLSurface surface = EGL_NO_SURFACE;
        EGLContext context = EGL_NO_CONTEXT;

        int w = 0;
        int h = 0;

        Pinch_Zoom_Recognizer zoom;
        Drag_Move_Recognizer move;

        int mouse_button_down[GLFW_MOUSE_BUTTON_LAST + 1]{};
        int key_down[GLFW_KEY_LAST + 1]{};
        std::unordered_map<int, int> hold_key_for_frames;

        Event_Slot<GLFWwindow*, int, int, int> mouse_button;
        Event_Slot<GLFWwindow*, double, double> mouse_move;

        GLFWwindow(ANativeWindow* win)
        {
            LOGI("Initializing display...");
            display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
            if (eglInitialize(display, 0, 0) == EGL_FALSE)
                throw std::runtime_error{"Failed to initialize EGL."};

            auto config = [&] {
                std::vector<EGLConfig> configs;
                EGLint config_count;
                EGLint attribs[] = {
                    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
                    EGL_BLUE_SIZE, 8,
                    EGL_GREEN_SIZE, 8,
                    EGL_RED_SIZE, 8,
                    EGL_DEPTH_SIZE, 24,
                    EGL_NONE,
                };

                eglChooseConfig(display, attribs, nullptr, 0, &config_count);
                configs.resize(config_count);
                eglChooseConfig(display, attribs, configs.data(), config_count, &config_count);
                configs.resize(config_count);
                assert(config_count > 0);

                LOGI("Got %d config(s).", (int)config_count);
                for (auto& cfg: configs) {
                    EGLint r;
                    EGLint g;
                    EGLint b;
                    EGLint d;
                    eglGetConfigAttrib(display, cfg, EGL_RED_SIZE, &r);
                    eglGetConfigAttrib(display, cfg, EGL_GREEN_SIZE, &g);
                    eglGetConfigAttrib(display, cfg, EGL_BLUE_SIZE, &b);
                    eglGetConfigAttrib(display, cfg, EGL_DEPTH_SIZE, &d);
                    LOGI("  Config r%d g%d b%d d%d", (int)r, (int)g, (int)b, (int)d);
                    if (r == 8 && g == 8 && b == 8 && d == 24)
                        return cfg;
                }

                LOGI("  Using FALLBACK config.");
                return configs[0];
            } ();

            LOGI("Creating surface...");
            surface = eglCreateWindowSurface(display, config, win, nullptr);
            if (surface == EGL_NO_SURFACE)
                throw std::runtime_error{"Failed to create EGL surface."};

            LOGI("Creating context...");
            context = [&] {
                EGLint attribs[] = {
                    EGL_CONTEXT_CLIENT_VERSION, 3,
                    EGL_NONE,
                };
                return eglCreateContext(display, config, EGL_NO_CONTEXT, attribs);
            } ();
            if (context == EGL_NO_CONTEXT)
                throw std::runtime_error{"Failed to create EGL context."};
        }

        ~GLFWwindow()
        {
            if (display == EGL_NO_DISPLAY) return;

            eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            if (context != EGL_NO_CONTEXT) eglDestroyContext(display, context);
            if (surface != EGL_NO_SURFACE) eglDestroySurface(display, surface);
            eglTerminate(display);
        }
    };

    int glfwInit()
    {
        clock_gettime(CLOCK_MONOTONIC, &init_time);
        glfw_initialized = true;
        return GLFW_TRUE;
    }

    void glfwTerminate()
    {
        glfw_initialized = false;
        glfw_windows.clear();
    }

    GLFWwindow* glfwCreateWindow(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share)
    {
        assert(glfw_initialized);
        assert(monitor == nullptr);
        assert(share == nullptr);

        glfw_windows.emplace_back(app->window);
        return &glfw_windows.back();
    }

    void glfwMakeContextCurrent(GLFWwindow* window)
    {
        assert(glfw_initialized);
        if (window) {
            if (eglMakeCurrent(window->display, window->surface, window->surface, window->context) == EGL_FALSE)
                throw std::runtime_error{"Failed to make EGL context current"};

            EGLint w;
            EGLint h;
            eglQuerySurface(window->display, window->surface, EGL_WIDTH, &w);
            eglQuerySurface(window->display, window->surface, EGL_HEIGHT, &h);
            window->w = w;
            window->h = h;

            LOGI("Vendor: %s", glGetString(GL_VENDOR));
            LOGI("Renderer: %s", glGetString(GL_RENDERER));
            LOGI("Version: %s", glGetString(GL_VERSION));

            int max_draw_buffer_count;
            glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max_draw_buffer_count);
            LOGI("Max draw buffer count = %d", max_draw_buffer_count);

            app->onInputEvent = glfw_handle_android_input;
        } else {
            eglMakeCurrent(ctx->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            app->onInputEvent = nullptr;
        }
        ctx = window;
    }

    void glfwPollEvents()
    {
        assert(glfw_initialized);
        assert(ctx);
        while (app->destroyRequested == 0) {
            ::android_poll_source* source{};
            if (ALooper_pollAll(0, nullptr, nullptr, (void**)&source) < 0) break;
            if (source) source->process(app, source);
        }
    }

    int32_t glfw_handle_android_input(::android_app* app, ::AInputEvent* ev)
    {
        assert(glfw_initialized);
        assert(ctx);

        if (AInputEvent_getType(ev) == AINPUT_EVENT_TYPE_MOTION) {
            if (auto zoom_amount = ctx->zoom.feed(ev)) {
                zoom_amount -= 1.0f;
                zoom_amount = zoom_amount * zoom_amount * zoom_amount * 1000.0f;
                zoom_amount += 1.0f;

                auto key = zoom_amount < 1.0f ? GLFW_KEY_S : GLFW_KEY_W;
                auto frames = std::abs(int((1.0f - 1.0f / zoom_amount) * 100.0f));
                if (frames > 60) frames = 60;

                LOGI("Zoom %.4f for %d frames", zoom_amount, frames);

                if (frames > 0) {
                    if (ctx->hold_key_for_frames[key] == 0)
                        ctx->key_down[key]++;
                    ctx->hold_key_for_frames[key] = frames;
                }
            }

            if (auto move_action = ctx->move.feed(ev)) {
                auto x = ctx->move.x();
                auto y = ctx->move.y();
                LOGI("Mouse at %.3f, %.3f", x, y);
                ctx->mouse_move.emit(ctx, x, y);

                if (move_action != 3) {
                    auto action = move_action > 0 ? GLFW_PRESS : GLFW_RELEASE;
                    auto button = std::abs(move_action) == 2 ? GLFW_MOUSE_BUTTON_LEFT : GLFW_MOUSE_BUTTON_RIGHT;
                    LOGI("Mouse %s %s", (button == GLFW_MOUSE_BUTTON_LEFT ? "left" : "right"), (action == GLFW_PRESS ? "down" : "up"));
                    ctx->mouse_button.emit(ctx, button, action, 0);
                    ctx->mouse_button_down[button] += (action == GLFW_PRESS ? 1 : -1);
                }
            }

            return 1;
        }

        return 0;
    }

    int glfwWindowShouldClose(GLFWwindow* window)
    {
        assert(glfw_initialized);
        assert(window);
        return (app->destroyRequested == 0 ? GLFW_FALSE : GLFW_TRUE);
    }

    void glfwSwapBuffers(GLFWwindow* window)
    {
        assert(glfw_initialized);
        assert(ctx);

        // Swap buffers
        if (eglSwapBuffers(ctx->display, ctx->surface) == EGL_FALSE)
            throw std::runtime_error{"Failed to swap buffers."};

        // Handle per-frame input emulation
        for (auto it=begin(ctx->hold_key_for_frames),last=end(ctx->hold_key_for_frames); it != last;) {
            if (it->second == 0) {
                it = ctx->hold_key_for_frames.erase(it);
            } else {
                ++it;
            }
        }
        for (auto& entry: ctx->hold_key_for_frames)
            if (--entry.second == 0)
                ctx->key_down[entry.first]--;
    }

    void glfwSwapInterval(int interval)
    {
        assert(glfw_initialized);
        assert(ctx);
        if (eglSwapInterval(ctx->display, interval) == EGL_FALSE)
            throw std::runtime_error{"Failed to set swap interval."};
    }

    void glfwGetFramebufferSize(GLFWwindow* window, int* width, int* height)
    {
        assert(glfw_initialized);
        assert(window);
        if (width) *width = window->w;
        if (height) *height = window->h;
    }

    void glfwGetWindowSize(GLFWwindow* window, int* width, int* height)
    {
        assert(glfw_initialized);
        assert(window);
        glfwGetFramebufferSize(window, width, height);

        // This will enable HiDPI support for IMGUI
        if (width) *width /= display_scaling;
        if (height) *height /= display_scaling;
    }

    double glfwGetTime()
    {
        assert(glfw_initialized);
        ::timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        auto sec = double(now.tv_sec - init_time.tv_sec);
        auto nano = double(now.tv_nsec - init_time.tv_nsec);
        return sec + nano * 1e-9;
    }

    int glfwGetKey(GLFWwindow* window, int key)
    {
        assert(glfw_initialized);
        assert(window);

        if (key < 0 || key > GLFW_KEY_LAST)
            throw std::runtime_error{"Invalid key code to glfwGetKey"};

        return (window->key_down[key] > 0 ? GLFW_PRESS : GLFW_RELEASE);
    }

    void glfwGetCursorPos(GLFWwindow* window, double* xpos, double* ypos)
    {
        assert(glfw_initialized);
        assert(window);
        if (xpos) *xpos = window->move.x();
        if (ypos) *ypos = window->move.y();
    }

    int glfwGetMouseButton(GLFWwindow* window, int button)
    {
        assert(glfw_initialized);
        assert(window);

        if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST)
            throw std::runtime_error{"Invalid mouse button to glfwGetMouseButton"};

        return (window->mouse_button_down[button] > 0 ? GLFW_PRESS : GLFW_RELEASE);
    }

    char const* glfwGetClipboardString(GLFWwindow* window)
    {
        assert(glfw_initialized);
        assert(window);
        return "";
    }

    void glfwWindowHint(int hint, int value)
    {
        assert(glfw_initialized);
        // empty intentionally
    }

    void glfwSetClipboardString(GLFWwindow* window, const char* string)
    {
        assert(glfw_initialized);
        assert(window);
        // empty intentionally
    }

    void glfwSetInputMode(GLFWwindow* window, int mode, int value)
    {
        assert(glfw_initialized);
        assert(window);
        // empty intentionally
    }

    GLFWcharfun glfwSetCharCallback(GLFWwindow* window, GLFWcharfun cbfun)
    {
        assert(glfw_initialized);
        assert(window);
        return glfw_stub_set_callback(cbfun);
    }

    GLFWcursorposfun glfwSetCursorPosCallback(GLFWwindow* window, GLFWcursorposfun cbfun)
    {
        assert(glfw_initialized);
        assert(window);
        return window->mouse_move.listen(cbfun);
    }

    GLFWkeyfun glfwSetKeyCallback(GLFWwindow* window, GLFWkeyfun cbfun)
    {
        assert(glfw_initialized);
        assert(window);
        return glfw_stub_set_callback(cbfun);
    }

    GLFWmousebuttonfun glfwSetMouseButtonCallback(GLFWwindow* window, GLFWmousebuttonfun cbfun)
    {
        assert(glfw_initialized);
        assert(window);
        return window->mouse_button.listen(cbfun);
    }

    GLFWscrollfun glfwSetScrollCallback(GLFWwindow* window, GLFWscrollfun cbfun)
    {
        assert(glfw_initialized);
        assert(window);
        return glfw_stub_set_callback(cbfun);
    }

    GLFWwindowfocusfun glfwSetWindowFocusCallback(GLFWwindow* window, GLFWwindowfocusfun cbfun)
    {
        assert(glfw_initialized);
        assert(window);
        // TODO
        return glfw_stub_set_callback(cbfun);
    }

    GLFWwindowiconifyfun glfwSetWindowIconifyCallback(GLFWwindow* window, GLFWwindowiconifyfun cbfun)
    {
        assert(glfw_initialized);
        assert(window);
        return glfw_stub_set_callback(cbfun);
    }
}

#endif

