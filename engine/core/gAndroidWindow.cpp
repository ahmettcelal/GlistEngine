/*
 * gAndroidWindow.cpp
 *
 *  Created on: June 24, 2023
 *      Author: Metehan Gezer
 */

#include "gAndroidWindow.h"
#include "gAppManager.h"

#ifdef ANDROID

static void onErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
	gLogi("gAndroidWindow") << "OpenGL ERROR: " << message;
}

ANativeWindow* gAndroidWindow::nativewindow = nullptr;
gAndroidWindow* window = nullptr;

gAndroidWindow::gAndroidWindow() {
    window = this;
    shouldclose = false;
	isclosed = true;
	isrendering = false;
}

gAndroidWindow::~gAndroidWindow() {
    close();
}

void gAndroidWindow::initialize(int uwidth, int uheight, int windowMode, bool isResizable) {
    gLogi("gAndroidWindow") << "initialize";
	const EGLint attribs[] = {
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT, // request OpenGL ES 3.0
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 16,
            EGL_NONE
	};
	EGLConfig config;
	EGLint numConfigs;
	EGLint format;

	if ((display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
		gLogi("gAndroidWindow") << "eglGetDisplay() returned error " << eglGetError();
        exit(-1);
		return;
	}
	if (!eglInitialize(display, 0, 0)) {
		gLogi("gAndroidWindow") << "eglInitialize() returned error " << eglGetError();
        exit(-1);
		return;
	}

	if (!eglChooseConfig(display, attribs, &config, 1, &numConfigs)) {
		gLogi("gAndroidWindow") << "eglChooseConfig() returned error " << eglGetError();
		close();
		return;
	}

	if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format)) {
		gLogi("gAndroidWindow") << "eglGetConfigAttrib() returned error " << eglGetError();
		close();
		return;
	}

	if (!(surface = eglCreateWindowSurface(display, config, nativewindow, 0))) {
		gLogi("gAndroidWindow") << "eglCreateWindowSurface() returned error " << eglGetError();
		close();
		return;
	}
    const EGLint attribList[] = {
			EGL_CONTEXT_CLIENT_VERSION, 3,
			EGL_NONE
	};

	if (!(context = eglCreateContext(display, config, EGL_NO_CONTEXT, attribList))) {
		gLogi("gAndroidWindow") << "eglCreateContext() returned error " << eglGetError();
		close();
		return;
	}

	if (!eglMakeCurrent(display, surface, surface, context)) {
		gLogi("gAndroidWindow") << "eglMakeCurrent() returned error " << eglGetError();
		close();
		return;
	}

	if (!eglQuerySurface(display, surface, EGL_WIDTH, &width) ||
		!eglQuerySurface(display, surface, EGL_HEIGHT, &height)) {
		gLogi("gAndroidWindow") << "eglQuerySurface() returned error " << eglGetError();
		close();
		return;
	}

	glViewport(0, 0, width, height);
	if(uwidth == 0) {
		uwidth = width;
	}
	if(uheight == 0) {
		uheight = height;
	}
	scalex = (float) width / (float) uwidth;
	scaley = (float) height / (float) uheight;
	isclosed = false;
	gBaseWindow::initialize(width, height, windowMode, false);
}


bool gAndroidWindow::getShouldClose() {
	return shouldclose || isclosed;
}

void gAndroidWindow::update() {
	if(!isrendering) {
		return;
	}

	if(!eglSwapBuffers(display, surface)) {
		gLogi("gAndroidWindow") << "eglSwapBuffers() returned error " << eglGetError();
	}
}

void gAndroidWindow::close() {
    if(!display) {
        return;
    }
    gLogi("gAndroidWindow") << "close";
	eglMakeCurrent(display,EGL_NO_SURFACE,EGL_NO_SURFACE, EGL_NO_CONTEXT );
	eglDestroySurface(display, surface);
	eglDestroyContext(display,context);
	eglTerminate(display);
	display = nullptr;
	isclosed = true;
}

void gAndroidWindow::setVsync(bool vsync) {
    gBaseWindow::setVsync(vsync);
}

void gAndroidWindow::setCursor(int cursorNo) {
}

void gAndroidWindow::setCursorMode(int cursorMode) {
}

void gAndroidWindow::setClipboardString(std::string text) {
}

std::string gAndroidWindow::getClipboardString() const {
	return "ASD";
}

void gAndroidWindow::setWindowSize(int width, int height) {
}

void gAndroidWindow::setWindowResizable(bool isResizable) {
}

void gAndroidWindow::setWindowSizeLimits(int minWidth, int minHeight, int maxWidth, int maxHeight) {
}

bool gAndroidWindow::isRendering() {
    return isrendering;
}

bool gAndroidWindow::onTouchCallback(int pointerCount, int* fingerIds, int* x, int* y) {
	TouchInput inputs[pointerCount];
	for (int i = 0; i < pointerCount; ++i) {
		inputs[i] = {fingerIds[i], i, x[i], y[i]};
	}
	gTouchEvent event{pointerCount, inputs};
	callEvent(event);
	return false;
}


extern "C" {
JNIEXPORT void JNICALL Java_dev_glist_android_lib_GlistNative_setSurface(JNIEnv *env, jclass clazz, jobject surface) {
	gLogi("GlistNative") << "setSurface";
	if (surface != nullptr) {
		gAndroidWindow::nativewindow = ANativeWindow_fromSurface(env, surface);
	} else {
		if(window) {
			window->close();
		}
		ANativeWindow_release(gAndroidWindow::nativewindow);
	}
}

JNIEXPORT void JNICALL Java_dev_glist_android_lib_GlistNative_onPause(JNIEnv *env, jclass clazz) {
    gLogi("GlistNative") << "onPause";
    if(window) {
    // todo pause event
        window->isrendering = false;
    }
}

JNIEXPORT void JNICALL Java_dev_glist_android_lib_GlistNative_onResume(JNIEnv *env, jclass clazz) {
    gLogi("GlistNative") << "onResume";
    if(window) {
    // todo resume event
        window->isrendering = true;
    }
}
}

#endif /* ANDROID */