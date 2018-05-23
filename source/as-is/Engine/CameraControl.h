#pragma once
#include "../../lib/gl.hpp"
#include "../../wsi/window-events.hpp"
#include "../../wsi/window-system.hpp"
#include "../../third-party/imgui/imgui-all.hpp"
#include "Camera.h"
#include <iostream>
#include <functional>
#include "../../oceanInfo/Camera/worldCamera.h"

//#include <windows.h>
using namespace HW;
using namespace std;


class CameraSpeed {
public:
	float speed_base = 0.1f;
	int level = 3;
	int MinLevel = 1;
	int MaxLevel = 10;
	float speed = 1.0f;

	CameraSpeed() {
		CalcSpeed();
	}

	void Dec(int d) {
		level -= d;
		if (level <= MinLevel)
			level = MinLevel;
		CalcSpeed();
	}
	void Inc(int d) {
		level += d;
		if (level >= MaxLevel)
			level = MaxLevel;
		CalcSpeed();
	}
	void CalcSpeed() {
		speed = speed_base*level*level*level;
	}
};

class CameraRotate {
public:
	//for camera rotate
	bool EnterRotateMode = false;
	bool firstMove = true;
	float lastx;
	float lasty;
	float RotateSpeed = 100;

	void process_mouse_button(int button, int  action) {
		if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
			EnterRotateMode = true;
			firstMove = true;
		}
		if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
			EnterRotateMode = false;
		}
	}
	void process_mouse_move(double x, double y, Camera* camera) {
		if (!EnterRotateMode) return;
		if (firstMove) {
			lastx = (float)x;
			lasty = (float)y;
			firstMove = false;
			return;
		}
		int w, h;
		glfwGetWindowSize(ss::Window_System::current().context(), &w, &h);
		float dx = (float(x) - lastx) / w*RotateSpeed;
		float dy = (float(y) - lasty) / h*RotateSpeed;
		lastx = (float)x;
		lasty = (float)y;
		camera->RotatePitchYaw(-dy, dx);
		camera->setUp(Vector3(0, 1, 0));
	}
};

class CameraMove {
public:
	CameraSpeed MoveSpeed;
	double LastTime;
	void move_func(Camera* camera) {
		auto frametime = ImGui::GetIO().DeltaTime;
		float d = MoveSpeed.speed * frametime;
		auto& ctx = ss::Window_System::current().context();
		if (glfwGetKey(ctx, GLFW_KEY_E) == GLFW_PRESS) {
			camera->MoveUp(d);
		}
		if (glfwGetKey(ctx, GLFW_KEY_Q) == GLFW_PRESS) {
			camera->MoveDown(d);
		}
		if (glfwGetKey(ctx, GLFW_KEY_W) == GLFW_PRESS) {
			camera->MoveForward(d);
		}
		if (glfwGetKey(ctx, GLFW_KEY_S) == GLFW_PRESS) {
			camera->MoveBack(d);
		}
		if (glfwGetKey(ctx, GLFW_KEY_A) == GLFW_PRESS) {
			camera->MoveLeft(d);
		}
		if (glfwGetKey(ctx, GLFW_KEY_D) == GLFW_PRESS) {
			camera->MoveRight(d);
		}
	}

	void move_speed_change(int key, int action) {
		if (key == GLFW_KEY_UP && action == GLFW_PRESS)
			MoveSpeed.Inc(1);
		if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
			MoveSpeed.Dec(1);
	}
};

class CameraFPS {
public:
	CameraMove cameraMove;
	CameraRotate cameraRotate;
	Camera* camera;
	int w, h;

	void Init(Camera* cam) {
		camera = cam;

		ss::window_events::mouse_button::subscribe([&] (auto, auto button, auto action, auto) {
			cameraRotate.process_mouse_button(button, action);
		});
		ss::window_events::mouse_move::subscribe([&] (auto, auto x, auto y) {
			cameraRotate.process_mouse_move(x, y, camera);
		});
		ss::window_events::key::subscribe([&] (auto, auto key, auto, auto action, auto) {
			cameraMove.move_speed_change(key, action);
		});
		ss::window_events::frame::subscribe([&] () {
			cameraMove.move_func(camera);
		});
	}
};

class GlobeInteractive
{
public:
	bool enableRotation;
	bool firstMove;
	double RotateSpeed;

    //DWORD lasttime;
    Vector2 mppos, lastpos;
    bool dragging;
    bool mouse_moved;
    bool mouse_pressed;
    double moveSpeed;
    double move_step;
    double moving_scale;
    VirtualGlobeScene::CameraInfo camera_info;
	struct mouse_event_struct
	{
		int mouse_button_left_state;
		int mouse_button_right_state;
		double xpos, ypos;
		mouse_event_struct() :mouse_button_left_state(GLFW_RELEASE), mouse_button_right_state(GLFW_RELEASE),
			xpos(0), ypos(0) {}
	} mouse_event;
	double clicked_longtitude, clicked_latitude;
    double target_longtitude, target_latitude;
    bool placeObj;
    int F_Type;

    bool left_mouse_pressed, right_mouse_pressed;
    bool left_mouse_just_release, right_mouse_just_release;
public:
    VirtualGlobeScene::MomentumCamera *camera;
    Ellipsoid *earthshape;
	RenderTarget *renderTarget;
    int w, h;
public:
	GlobeInteractive(int w, int h, VirtualGlobeScene::MomentumCamera *camera = NULL, Ellipsoid *earthshape = NULL)
		:w(w), h(h), camera(camera), earthshape(earthshape),
		enableRotation(false), firstMove(false), RotateSpeed(100), mppos(0, 0), lastpos(0, 0),
		dragging(false),mouse_moved(false),mouse_pressed(false), moveSpeed(100), move_step(100), moving_scale(1),
		camera_info(), mouse_event(), clicked_longtitude(0), clicked_latitude(0), target_longtitude(0), target_latitude(0), F_Type(0),
        placeObj(false), left_mouse_pressed(false), right_mouse_pressed(false), left_mouse_just_release(false), right_mouse_just_release(false)
	{

	}
    void getMoustEvent()
    {
        auto& ctx = ss::Window_System::current().context();
        mouse_event.mouse_button_left_state = glfwGetMouseButton(ctx, GLFW_MOUSE_BUTTON_LEFT);
        mouse_event.mouse_button_right_state = glfwGetMouseButton(ctx, GLFW_MOUSE_BUTTON_RIGHT);
        glfwGetCursorPos(ctx, &mouse_event.xpos, &mouse_event.ypos);
    }
	void perFrameInteractive()
	{
		moveGlobe(camera);
		rotateGlobe(camera);
	}
	void moveGlobe(VirtualGlobeScene::MomentumCamera *camera)
	{
		auto frametime = ImGui::GetIO().DeltaTime;
		float d = moveSpeed * frametime;
		auto& ctx = ss::Window_System::current().context();
		if (glfwGetKey(ctx, GLFW_KEY_W) == GLFW_PRESS) {
			camera->zoomStepped(d);
		}
		if (glfwGetKey(ctx, GLFW_KEY_S) == GLFW_PRESS) {
			camera->zoomStepped(-d);
		}
        if (glfwGetKey(ctx, GLFW_KEY_X) == GLFW_PRESS) {
            placeObj = true;
        }
        if (glfwGetKey(ctx, GLFW_KEY_F1) == GLFW_PRESS) {
            //Speed mode
            //F_Type = 1;
        }
        if (glfwGetKey(ctx, GLFW_KEY_F2) == GLFW_PRESS) {
            //Temp mode
            //F_Type = 2;
        }
        if (glfwGetKey(ctx, GLFW_KEY_F3) == GLFW_PRESS) {
            //Pressure mode
            //F_Type = 3;
        }

        //need add more

	}
	void rotateGlobe(VirtualGlobeScene::MomentumCamera *camera)
	{
        getMoustEvent();
        processMousePressEvent();
        processMouseMoveEvent();
	}
    void processMousePressEvent()
    {
        /*if (!hasFocus()) setFocus();

        if (action_plugin)
        {
            action_plugin->mousePressEvent(evt);
            if (action_plugin->is_action_handled())
                return;
        }*/
        if (true)
        {
            if (mouse_event.mouse_button_left_state == GLFW_PRESS && left_mouse_pressed == false)
            {
                dragging = false;
                left_mouse_pressed = true; left_mouse_just_release = false;
                mppos.x = lastpos.x = mouse_event.xpos;
                mppos.y = lastpos.y = mouse_event.ypos;
                // cout << "left mouse just pressed" << endl;
            }
            if (mouse_event.mouse_button_right_state == GLFW_PRESS && right_mouse_pressed == false)
            {
                dragging = false;
                right_mouse_pressed = true; right_mouse_just_release = false;
                mppos.x = lastpos.x = mouse_event.xpos;
                mppos.y = lastpos.y = mouse_event.ypos;
            }
            if (mouse_event.mouse_button_left_state == GLFW_PRESS && left_mouse_pressed == true)
            {
                // mppos.x = lastpos.x = mouse_event.xpos;
                 //mppos.y = lastpos.y = mouse_event.ypos;
               // cout << "left mouse keep pressed" << endl;
            }
            if (mouse_event.mouse_button_right_state == GLFW_PRESS && right_mouse_pressed == true)
            {
                // mppos.x = lastpos.x = mouse_event.xpos;
                // mppos.y = lastpos.y = mouse_event.ypos;
            }
            if (mouse_event.mouse_button_left_state == GLFW_RELEASE && left_mouse_pressed == false)
            {
                // cout << "left mouse keep released" << endl;
                left_mouse_just_release = false;
                // dragging = false;
            }
            if (mouse_event.mouse_button_right_state == GLFW_RELEASE && right_mouse_pressed == false)
            {
                right_mouse_just_release = false;
                // dragging = false;
            }
            if (mouse_event.mouse_button_left_state == GLFW_RELEASE && left_mouse_pressed == true)
            {
                //cout << "left mouse just released" << endl;
                left_mouse_pressed = false;
                left_mouse_just_release = true;
            }
            if (mouse_event.mouse_button_right_state == GLFW_RELEASE && right_mouse_pressed == true)
            {
                right_mouse_pressed = false;
                right_mouse_just_release = true;
            }
        }
        /*if (mouse_event.mouse_button_left_state == GLFW_PRESS || mouse_event.mouse_button_right_state == GLFW_PRESS)
        {
            if (mouse_pressed == false)
            {
                mppos.x = lastpos.x = mouse_event.xpos;
                mppos.y = lastpos.y = mouse_event.ypos;
                camera->set_camera_has_momentum(false);
            }
            mouse_pressed = true;
        }
        else
        {
            mouse_pressed = false;
        }*/
        bool onWeb = false;
        if (onWeb == false)
        {
            if (left_mouse_just_release && !dragging)
            {
                double lat, log;
               // if (camera->pickingRayIntersection(lastpos.x, height() - lastpos.y - 1, lat, log))
                if (computeIntersection(lastpos.x, height() - lastpos.y - 1, lat, log))
                {
                    camera->pointGoto(lat, log);
                }
                clicked_longtitude = log;
                clicked_latitude = lat;
                if (!placeObj)
                {
                    target_longtitude = clicked_longtitude;
                    target_latitude = clicked_latitude;
                }
                cout << "(lat, log)->" << degrees(lat) << "," << degrees(log) << endl;
            }

        }
        else
        {
            if (mouse_event.mouse_button_left_state == GLFW_PRESS)
            {
                double lat, log;
                //if (camera->pickingRayIntersection(mouse_event.xpos, height() - mouse_event.ypos - 1, lat, log))
				cout << "mouse_event_pos = (" << mouse_event.xpos << "," << mouse_event.ypos << ")" << endl;

				if (computeIntersection(mouse_event.xpos, height() - mouse_event.ypos - 1, lat, log))
				{
                    camera->pointGoto(lat, log);
                }
                clicked_longtitude = log;
                clicked_latitude = lat;
				cout << "check Point3 " << endl;
                if (!placeObj)
                {
                    target_longtitude = clicked_longtitude;
                    target_latitude = clicked_latitude;
                }
                cout << "mouse_event_pos = (" << mouse_event.xpos << "," << mouse_event.ypos << ")" << endl;
                cout << "(lat, log)->" << degrees(lat) << "," << degrees(log) << endl;
            }
        }
        //[IGN] plugin actived and take control
        //      send mousePressEvent to plugin
        /*if (evt->button() == Qt::LeftButton) _lbtn = true;
        else if (evt->button() == Qt::RightButton) _rbtn = true;
        else if (evt->button() == Qt::MidButton) _mbtn = true;*/
        // clear _mouse_moved state
       // mouse_moved = false;
    }
    void processMouseMoveEvent()
    {
       /* if (!hasFocus()) setFocus();

        if (action_plugin)
        {
            action_plugin->mouseMoveEvent(evt);
            if (action_plugin->is_action_handled())
                return;
        }*/
        //cout << "Global "<<(camera->csys() == VirtualGlobeScene::CoordinateSystem::GLOBAL) << endl;
        //cout << "b_trackball " << (camera->b_trackball()) << endl;
        if (camera->csys() == VirtualGlobeScene::CoordinateSystem::GLOBAL && !camera->b_trackball())
        {
            //cout << "GLOBAL, not trackball" << endl;
            //[NOTE] normalize do NOT modify itself
            int flag = 0;
            int diff;
            vec3d up = camera->up();
            vec3d firstViewDir = camera->target() - camera->position();
            firstViewDir = firstViewDir.normalize();
            vec3d firstViewLeft = up.crossProduct(firstViewDir).normalize();
            vec3d firstViewUp = up.normalize();

            camera_info.firstview_offset = vec3d(0.0f, 0.0f, 0.0f);

            float scale = (fabs(camera->altitude())) / 500;
			scale = scale < 1.0 ? 1.0 : scale;
			scale = scale > 10000.0 ? 10000.0 : scale;
            if (mouse_event.mouse_button_left_state== GLFW_PRESS && mouse_event.mouse_button_right_state!= GLFW_PRESS)
            {
                diff = lastpos.x - mouse_event.xpos;
                if (diff != 0)
                {
                    camera_info.firstview_offset = -firstViewLeft * diff * scale;
                }
                diff = mouse_event.ypos - lastpos.y;
                if (diff != 0)
                {
                    camera_info.firstview_offset += firstViewDir * diff * scale;
                }

                flag = 1;
            }
            else if (mouse_event.mouse_button_right_state== GLFW_PRESS && mouse_event.mouse_button_left_state!= GLFW_PRESS)
            {
                diff = lastpos.x - mouse_event.xpos;
                if (diff != 0)
                {
                    camera_info.eye_to_target += -firstViewLeft * diff / 500;
                }
                diff = mouse_event.ypos - lastpos.y;
                if (diff != 0)
                {
                    camera_info.eye_to_target += firstViewUp * diff / 500;
                }

                camera_info.eye_to_target = camera_info.eye_to_target.normalize();
                flag = 1;
            }

            lastpos.x = mouse_event.xpos;
            lastpos.y = mouse_event.ypos;
            if (flag)
            {
                camera->setEye(camera->position() + camera_info.firstview_offset);
                camera->setTarget(camera->position() + camera_info.eye_to_target);
                vec3d p1 = camera->position();
                vec3d p2 = camera_info.eye_to_target;
                up = earthshape->GeodeticSurfaceNormal(camera->position());
                camera->setUp(up);
                camera->computeTiltAndHeading();
            }
            // mouse moved since last press
            mouse_moved = true;
            return;
        }

        ///it seems that coordinate system of main camera can not be local in current version.
        if (camera->csys() == VirtualGlobeScene::LOCAL)
        {
            return;
        //    int flag = 0;
        //    int diff;
        //    vec3d up = camera_->local_up();
        //    vec3d firstViewDir = camera_->local_dir();
        //    vec3d firstViewLeft = up.crossProduct(firstViewDir).normalize();
        //    vec3d firstViewUp = up.normalize();

        //    cam_info_.firstview_offset = vec3d(0.0f, 0.0f, 0.0f);
        //    cam_info_.eye_to_target = camera_->target_dir();

        //    float scale = (fabs(camera_->local_eye().z)) / 1200;
        //    scale = scale<0.3 ? 0.3 : scale;
        //    if (_lbtn && !_rbtn)
        //    {
        //        diff = _lastpos.x - evt->x();
        //        if (diff != 0)
        //        {
        //            cam_info_.firstview_offset = -firstViewLeft * diff * scale;
        //        }
        //        diff = evt->y() - _lastpos.y;
        //        if (diff != 0)
        //        {
        //            vec3d tmp = firstViewDir;
        //            tmp.z = 0;
        //            tmp = tmp.normalize();
        //            cam_info_.firstview_offset += tmp * diff * scale;
        //        }

        //        flag = 1;
        //    }
        //    else if (_rbtn && !_lbtn)
        //    {
        //        diff = _lastpos.x - evt->x();
        //        if (diff != 0)
        //        {
        //            cam_info_.eye_to_target += -firstViewLeft * diff / 500;
        //        }
        //        diff = evt->y() - _lastpos.y;
        //        if (diff != 0)
        //        {
        //            if (camera_->tilt() < 15.0*M_PI / 180)
        //                diff *= 4;

        //            cam_info_.eye_to_target += firstViewUp * diff / 150;
        //        }

        //        cam_info_.eye_to_target = cam_info_.eye_to_target.normalize();
        //        flag = 1;
        //    }

        //    _lastpos.x = evt->x();
        //    _lastpos.y = evt->y();
        //    if (flag)
        //    {
        //        camera_->set_target_dst(camera_->target_dst() + cam_info_.firstview_offset*moving_scale_);
        //        camera_->set_target_dir(camera_->target_dir() + cam_info_.eye_to_target);
        //    }
        //    // mouse moved since last press
        //    _mouse_moved = true;
        //    return;
        }

        //Global coordinate system and enable trailball
        int dx = mouse_event.xpos - lastpos.x;
        int dy = mouse_event.ypos - lastpos.y;
        //cout << "dx: " << dx << endl;
        //cout << "dy: " << dy << endl;
        float speed = 0;
        float frametime = ImGui::GetIO().DeltaTime;
        //DWORD curtime = GetTickCount(); //[NOTE] replace with stardard timer
        //if (curtime > lasttime)
        //  speed = sqrt((float)(dx*dx + dy * dy)) * 1000.0f / (curtime - lasttime);  // pixels per second
        //lasttime = curtime;
        speed = sqrt((float)(dx*dx + dy * dy)) * 1000.0f / frametime;
        if ((mouse_event.mouse_button_left_state == GLFW_PRESS || mouse_event.mouse_button_right_state == GLFW_PRESS)
            && (dx*dx + dy * dy > 9))  // dist > 3 pixels
        {
            dragging = true;
        }
        if (mouse_event.mouse_button_left_state == GLFW_PRESS && mouse_event.mouse_button_right_state != GLFW_PRESS)
        {
            cout << "Global, trackball, left/not right" << endl;
            float scale = std::max<float>(1.0f, (speed / 3000.0));
            double prelat, prelog;
           // bool prehit = camera->pickingRayIntersection(lastpos.x, height() - lastpos.y - 1,
            //    prelat, prelog);
			bool prehit = computeIntersection(lastpos.x, height() - lastpos.y - 1,
				prelat, prelog);
            double lat, log;

            if (!camera->underground())
            {
               // bool hit = camera->pickingRayIntersection(mouse_event.xpos, height() - mouse_event.ypos - 1, lat,
               //     log);
				bool hit = computeIntersection(mouse_event.xpos, height() - mouse_event.ypos - 1, lat,
					log);
                if (hit && prehit && false) {
                    double alti_at = camera->altitudeAboveTerrain();
                    double alti = camera->altitude();
                    double latlimit = -1.0;
                    if (alti_at < 6000) {
                        move_step = 1.0 + (alti_at - 6000) * 0.6 / 6000;
                        latlimit = 0.1 * ((alti + 100) / 110000) * radians(1.0);
                        if (alti_at < 600 && camera->tilt() > radians(45.0))
                            latlimit *= (0.005 + cos(camera->tilt()) / 4);
                    }
                    double dlat = prelat - lat;
                    double dlog = prelog - log;

                    double loglimit = 1.5 * fabs(dlog * latlimit / dlat);
                    if ((prelog * log < 0) && (fabs(dlog) > M_PI)) {
                        if (dlog > 0) dlog -= 2 * M_PI;
                        else dlog += 2 * M_PI;
                    }
                    if (fabs(dlog) * scale - M_PI > -1e-3)
                        scale = (M_PI - 1e-3) / fabs(dlog);
                    double deltaX = dlat * move_step;
                    double deltaY = dlog * move_step;
                    if (latlimit > 0) {
                        if (fabs(deltaX) > latlimit)
                            deltaX = deltaX > 0 ? latlimit : -latlimit;
                        if (fabs(deltaY) > loglimit)
                            deltaY = deltaY > 0 ? loglimit : -loglimit;
                    }
                    camera->pan(deltaX * scale *moving_scale, deltaY * scale *moving_scale);
                }
                else
                {
                    double dlat = (dy * camera->altitude()) / (800 * earthshape->_radii.z);
                    double dlog = (-dx * camera->altitude()) / (800 * earthshape->_radii.z);
                    if (fabs(camera->heading()) > M_PI / 2)
                        dlog = -dlog;
                    if (fabs(dlog) - M_PI > -1e-2) {
                        if (dlog > 0) dlog = M_PI - 1e-2;
                        else dlog = 1e-2 - M_PI;
                    }
                    camera->pan(dlat * moving_scale, dlog * moving_scale);
                }
            }
            else
            {
                int tmp = (camera->heading() <= 0) ? 1 : -1;
                int tmp1 = ((camera->heading() - M_PI / 2) <= 0) ? 1 : -1;
                double sscale = fabs(camera->altitude());
                if (camera->height_above_terrian() < 0)
                {
                    sscale = fabs(camera->altitude()) * 10;
                    if (sscale < 80) sscale = 80;
                }

                double deltaY = (double)(dy * sscale) / (800 * earthshape->_radii.z);
                double deltaX = (double)(dx * sscale) / (800 * earthshape->_radii.z);

                double deltaLat = deltaY * cos(camera->heading())
                    + deltaX * cos(camera->heading() - M_PI / 2);
                double deltaLon = deltaY * fabs(sin(camera->heading())) * tmp
                    + deltaX * fabs(sin(camera->heading() - M_PI / 2)) * tmp1;

                deltaLon = -deltaLon;
                if (fabs(deltaLon) > M_PI - 1e-2)
                {
                    if (deltaLon > 0)
                        deltaLon = M_PI - 1e-2;
                    else
                        deltaLon = 1e-2 - M_PI;
                }
                camera->pan(deltaLat * moving_scale, deltaLon * moving_scale);
            }
        }
        else if (mouse_event.mouse_button_right_state == GLFW_PRESS && mouse_event.mouse_button_left_state != GLFW_PRESS)
        {
           // cout << "Global, trackball, not left/right" << endl;
            float dxn = (float)dx / width();
            float dyn = (float)dy / height();
            cout << dxn << " " << dyn << endl;
            camera->rotationYawPitchRoll(0, 0, -dxn * 3.5);
            camera->set_target_tilt(camera->target_tilt() + dyn * 3.5);
        }

        // update last pos
        lastpos.x = mouse_event.xpos;
        lastpos.y = mouse_event.ypos;
        // mouse moved since last press
        mouse_moved = true;
    }
    float  height()
    {
        return h;
    }
    float  width()
    {
        return w;
    }
    bool computeIntersection(int screenx, int screeny, double &latitude, double &longitude)
    {
        double xpos = double(screenx) / width();
        double ypos = double(screeny) / height();
        vec3d oneOverEllipsoidRadiiSquared = earthshape->_oneOverRadiiSquared;
        vec3d rayOrigin = camera->position();
        vec3d rayOriginSquared = camera->position() * camera->position();
        vec3d direction = camera->getDir();
        vec3d up = camera->getCameraUpVector();
        vec3d horizontal = direction.crossProduct(up);
        double fov = camera->fovx();
        double aspect = camera->aspect();
        double distToZ = 0.5 / tan(fov / 2.0f);
        vec3d rayDirection = direction * distToZ + horizontal * (xpos - 0.5) + up * (ypos - 0.5)/ aspect;

        float a = oneOverEllipsoidRadiiSquared.dotproduct(rayDirection * rayDirection);
        //dot(rayDirection * rayDirection, oneOverEllipsoidRadiiSquared);
        float b = 2.0 *oneOverEllipsoidRadiiSquared.dotproduct(rayOrigin * rayDirection);
        //dot(rayOrigin * rayDirection, oneOverEllipsoidRadiiSquared);
        float c = oneOverEllipsoidRadiiSquared.dotproduct(rayOriginSquared) - 1.0;
        //dot(rayOriginSquared, oneOverEllipsoidRadiiSquared) - 1.0;
        float discriminant = b * b - 4.0 * a * c;

        if (discriminant < 0.0)
        {
            latitude = 0; longitude = 0;
            return false;
        }

        float nearz = 0.0, farz = 0.0;
        if (discriminant == 0.0)
        {
            float time = -0.5 * b / a;
            nearz = time; farz = time;
        }
        else
        {
            float t = -0.5 * (b + (b > 0.0 ? 1.0 : -1.0) * sqrt(discriminant));
            float root1 = t / a;
            float root2 = c / t;
            nearz = min(root1, root2);
            farz = max(root1, root2);
        }
        vec3d position = rayOrigin + rayDirection * nearz;
        Geodetic3D i1t = earthshape->ToGeodetic3D(position);
        latitude = i1t.getLatitude();
        longitude = i1t.getLongitude();
        return true;
    }
};


