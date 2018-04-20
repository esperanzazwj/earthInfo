MAKEFLAGS += --no-print-directory

.PHONY: all run
all: build/current-target
	@$(MAKE) $$(<$<)
run: build/current-target
	@$(MAKE) run-$$(<$<)
build/current-target: | build/
	@$(MAKE) target-linux

.PHONY: clean reset rebuild
clean:
	rm -rf build/
	rm -rf scripts/android/build/
	rm -rf scripts/android/.externalNativeBuild/
	rm -f bin/ss-engine
	rm -f bin/ss-engine.{html,js,wasm,data}
reset:
	rm -f build/current-target
rebuild:
	@$(MAKE) clean
	@$(MAKE) all

.PHONY: linux run-linux
linux: target-linux
	@$(MAKE) build-linux
run-linux: linux
	cd bin && ./ss-engine
build-linux: | build/linux/
	cd build/linux && cmake ../..
	$(MAKE) -C build/linux

.PHONY: web run-web
web: target-web
	@$(MAKE) build-web
run-web: web
	source /etc/profile && emrun --no_browser --no_emrun_detect bin/ss-engine.html
build-web: | build/web/
	source /etc/profile && cd build/web && emcmake cmake ../..
	source /etc/profile && $(MAKE) -C build/web

.PHONY: android run-android
android: target-android
	@$(MAKE) build-android
run-android: android
	cd scripts/android && bash gradlew installDebug
	adb logcat
build-android:
	cd scripts/android && bash gradlew assembleDebug

build/:
build/web/: | build/
build/linux/: | build/

%/:
	mkdir $@

target-%: | build/
	@echo $(@:target-%=%) > build/current-target

