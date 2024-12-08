#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <numbers>
#include <typeinfo>
#include <stdexcept>

#include <cstdio>
#include <cstdlib>

#include "../support/error.hpp"
#include "../support/program.hpp"
#include "../support/checkpoint.hpp"
#include "../support/debug_output.hpp"

#include "../vmlib/vec3.hpp"
#include "../vmlib/vec4.hpp"
#include "../vmlib/mat44.hpp"
#include "../vmlib/mat33.hpp"

#include "defaults.hpp"
#include "loadobj.hpp"

namespace
{
	constexpr char const* kWindowTitle = "COMP3811 - CW2";

	constexpr float kMovementPerSecond_ = 5.f; // units per second
    constexpr float kMouseSensitivity_ = 0.1f; // radians per pixel

    float fbwidth, fbheight;
 
    // This will contain the state of our program
    struct State_ {
        ShaderProgram* prog;

        struct CamCtrl_ {
            bool cameraActive;

			bool actionMoveForward;
			bool actionMoveBackward;
			bool actionMoveLeft;
			bool actionMoveRight;
			bool actionMoveUp;
			bool actionMoveDown;

			Vec3f position;

            float pitch;    // Looking left / right
            float yaw;      // Looking up / down
			

        } camControl;

        double deltaTime;    // This allows smooth camera movement
    };
	
	void glfw_callback_error_( int, char const* );

	void glfw_callback_key_( GLFWwindow*, int, int, int, int );
    void glfw_callback_motion_( GLFWwindow* aWindow, double aMouseXPos, double aMouseYPos );
    void glfw_callback_mouse_( GLFWwindow* aWindow, int aButton, int aAction, int aMods );

	struct GLFWCleanupHelper
	{
		~GLFWCleanupHelper();
	};
	struct GLFWWindowDeleter
	{
		~GLFWWindowDeleter();
		GLFWwindow* window;
	};

}

int main() try
{
	// Initialize GLFW
	if( GLFW_TRUE != glfwInit() )
	{
		char const* msg = nullptr;
		int ecode = glfwGetError( &msg );
		throw Error( "glfwInit() failed with '%s' (%d)", msg, ecode );
	}

	// Ensure that we call glfwTerminate() at the end of the program.
	GLFWCleanupHelper cleanupHelper;

	// Configure GLFW and create window
	glfwSetErrorCallback( &glfw_callback_error_ );

	glfwWindowHint( GLFW_SRGB_CAPABLE, GLFW_TRUE );
	glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );

	//glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );
	
#	if !defined(__APPLE__)
	// Most platforms will support OpenGL 4.3
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
#	else // defined(__APPLE__)
	// Apple has at most OpenGL 4.1, so don't ask for something newer.
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
#	endif // ~ __APPLE__
	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

	//TODO: additional GLFW configuration here
	glfwWindowHint( GLFW_DEPTH_BITS, 24 );

#	if !defined(NDEBUG)
	// When building in debug mode, request an OpenGL debug context. This
	// enables additional debugging features. However, this can carry extra
	// overheads. We therefore do not do this for release builds.
	glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE );
#	endif // ~ !NDEBUG

	GLFWwindow* window = glfwCreateWindow(
		1280,
		720,
		kWindowTitle,
		nullptr, nullptr
	);

	if( !window )
	{
		char const* msg = nullptr;
		int ecode = glfwGetError( &msg );
		throw Error( "glfwCreateWindow() failed with '%s' (%d)", msg, ecode );
	}

	GLFWWindowDeleter windowDeleter{ window };

	// Set up event handling
	// TODO: Additional event handling setup
	glfwSetKeyCallback( window, &glfw_callback_key_ );
    glfwSetCursorPosCallback( window, &glfw_callback_motion_ );
    glfwSetMouseButtonCallback( window, &glfw_callback_mouse_ );

    State_ state {};

    // This allows us to access 'state' without making it global
    glfwSetWindowUserPointer( window, &state );


	// Set up drawing stuff
	glfwMakeContextCurrent( window );
	glfwSwapInterval( 1 ); // V-Sync is on.

	// Initialize GLAD
	// This will load the OpenGL API. We mustn't make any OpenGL calls before this!
	if( !gladLoadGLLoader( (GLADloadproc)&glfwGetProcAddress ) )
		throw Error( "gladLoaDGLLoader() failed - cannot load GL API!" );

	std::printf( "RENDERER %s\n", glGetString( GL_RENDERER ) );
	std::printf( "VENDOR %s\n", glGetString( GL_VENDOR ) );
	std::printf( "VERSION %s\n", glGetString( GL_VERSION ) );
	std::printf( "SHADING_LANGUAGE_VERSION %s\n", glGetString( GL_SHADING_LANGUAGE_VERSION ) );

	// Ddebug output
#	if !defined(NDEBUG)
	setup_gl_debug_output();
#	endif // ~ !NDEBUG

	// Global GL state
	OGL_CHECKPOINT_ALWAYS();

	// TODO: global GL setup goes here
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_FRAMEBUFFER_SRGB );
	glEnable( GL_CULL_FACE );
	glClearColor( 0.2f, 0.2f, 0.2f, 0.f );

	OGL_CHECKPOINT_ALWAYS();

	// Get actual framebuffer size.
	// This can be different from the window size, as standard window
	// decorations (title bar, borders, ...) may be included in the window size
	// but not be part of the drawable surface area.
	int iwidth, iheight;
	glfwGetFramebufferSize( window, &iwidth, &iheight );

	float wscale = 1.f, hscale = 1.f;
#   if defined(__APPLE__)
    	glfwGetWindowContentScale( window, &wscale, &hscale );
#   endif
    iwidth = int(iwidth/wscale);
    iheight = int(iheight/hscale);

	glViewport( 0, 0, iwidth, iheight );

	// Load shader program
	ShaderProgram prog( {
		{ GL_VERTEX_SHADER, "assets/cw2/default.vert" },
		{ GL_FRAGMENT_SHADER, "assets/cw2/default.frag" }
	} );

	// FIX FOR 4.1
	GLint uProjCameraWorldLocation = glGetUniformLocation(prog.programId(), "uProjCameraWorld");
	GLint uNormalMatrixLocation = glGetUniformLocation(prog.programId(), "uNormalMatrix");
	GLint uLightDirLocation = glGetUniformLocation(prog.programId(), "uLightDir");
	GLint uLightDiffuseLocation = glGetUniformLocation(prog.programId(), "uLightDiffuse");
	GLint uSceneAmbientLocation = glGetUniformLocation(prog.programId(), "uSceneAmbient");

	// Ensure the locations are valid
	if (uProjCameraWorldLocation == -1 || uNormalMatrixLocation == -1 || uLightDirLocation == -1 || uLightDiffuseLocation == -1 || uSceneAmbientLocation == -1) {
		std::fprintf(stderr, "Error: Uniform location not found\n");
	}

    state.prog = &prog;

    state.camControl.cameraActive = false;
    state.camControl.pitch = 0.f;
    state.camControl.yaw = 0.f;
    state.deltaTime = 0.1f;
	state.camControl.position = { 0.f, 0.f, 0.f };

	// Other initialization & loading
	OGL_CHECKPOINT_ALWAYS();

    // Load the terrain and add to VAO
    auto langersoMesh = load_wavefront_obj("assets/cw2/langerso.obj");
    GLuint vao = create_vao(langersoMesh);
    std::size_t vertexCount = langersoMesh.positions.size();

    double last = glfwGetTime();

	// Main loop
	while( !glfwWindowShouldClose( window ) )
	{
		// Let GLFW process events
		glfwPollEvents();
		
		// Check if window was resized.
		{
			int nwidth, nheight;
			glfwGetFramebufferSize( window, &nwidth, &nheight );

			fbwidth = float(nwidth);
			fbheight = float(nheight);

			if( 0 == nwidth || 0 == nheight )
			{
				// Window minimized? Pause until it is unminimized.
				// This is a bit of a hack.
				do
				{
					glfwWaitEvents();
					glfwGetFramebufferSize( window, &nwidth, &nheight );
				} while( 0 == nwidth || 0 == nheight );
			}

			glViewport( 0, 0, nwidth, nheight );
		}

		// Update state
        double currentTime = glfwGetTime();
        state.deltaTime = currentTime - last;
        last = currentTime;

		//TODO: update state
        Mat44f model2world = kIdentity44f;


		Mat44f Rx = make_rotation_x( state.camControl.pitch );
        Mat44f Ry = make_rotation_y( state.camControl.yaw );


		// have to somehow make it so when w pressed it moves in the cameras direction
		// Use yaw and pitch to calculate the direction the camera is facing
		// store the facing direction in CAMCONTROL???
		float velocity = kMovementPerSecond_ * state.deltaTime;

		if (state.camControl.actionMoveUp) {
			state.camControl.position.y += velocity;
		}
		if (state.camControl.actionMoveDown) {
			state.camControl.position.y -= velocity;
		}

		// negative as you move the world around the camera
        Mat44f T = make_translation( -state.camControl.position );

		
        Mat44f world2camera = T * Rx * Ry;


        Mat44f projection = make_perspective_projection(
            60.f * std::numbers::pi_v<float> / 180.f,
            fbwidth / float(fbheight),
            0.1f, 100.0f
        );

        Mat44f projCameraWorld = projection * world2camera * model2world;
        Mat33f normalMatrix = mat44_to_mat33(model2world);

		// Draw scene
		OGL_CHECKPOINT_DEBUG();

		//TODO: draw frame
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glUseProgram( prog.programId() );

		glUniformMatrix4fv(
			uProjCameraWorldLocation,
			1, GL_TRUE, projCameraWorld.v
		);

		glUniformMatrix3fv(
			uNormalMatrixLocation,
			1, GL_TRUE, normalMatrix.v
		);

        Vec3f lightDir = normalize( Vec3f{ 0.f, 1.f, -1.f } );

		glUniform3fv( uLightDirLocation, 1, &lightDir.x );
		glUniform3f( uLightDiffuseLocation, 0.9f, 0.9f, 0.6f );
		glUniform3f( uSceneAmbientLocation, 0.05f, 0.05f, 0.05f );

        glBindVertexArray( vao );
        glDrawArrays( GL_TRIANGLES, 0, vertexCount );

		OGL_CHECKPOINT_DEBUG();

		// Display results
		glfwSwapBuffers( window );
	}

	// Cleanup.
	//TODO: additional cleanup
    glBindVertexArray( 0 );
    glUseProgram( 0 );
	
	return 0;
}
catch( std::exception const& eErr )
{
	std::fprintf( stderr, "Top-level Exception (%s):\n", typeid(eErr).name() );
	std::fprintf( stderr, "%s\n", eErr.what() );
	std::fprintf( stderr, "Bye.\n" );
	return 1;
}


namespace
{
	void glfw_callback_error_( int aErrNum, char const* aErrDesc )
	{
		std::fprintf( stderr, "GLFW error: %s (%d)\n", aErrDesc, aErrNum );
	}

	void glfw_callback_key_( GLFWwindow* aWindow, int aKey, int, int aAction, int )
	{
		if( GLFW_KEY_ESCAPE == aKey && GLFW_PRESS == aAction )
		{
			glfwSetWindowShouldClose( aWindow, GLFW_TRUE );
			return;
		}

		if( auto* state = static_cast<State_*>(glfwGetWindowUserPointer( aWindow )) )
		{
			// Camera controls if camera is active
			if( state->camControl.cameraActive )
			{
				if (GLFW_KEY_W == aKey) {
					if (GLFW_PRESS == aAction)
						state->camControl.actionMoveForward = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.actionMoveForward = false;
				}
				else if (GLFW_KEY_A == aKey) {
					if (GLFW_PRESS == aAction)
						state->camControl.actionMoveLeft = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.actionMoveLeft = false;
				}
				else if (GLFW_KEY_S == aKey) {
					if (GLFW_PRESS == aAction)
						state->camControl.actionMoveBackward = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.actionMoveBackward = false;
				}
				else if (GLFW_KEY_D == aKey) {
					if (GLFW_PRESS == aAction)
						state->camControl.actionMoveRight = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.actionMoveRight = false;
				}
				else if (GLFW_KEY_E == aKey) {
					if (GLFW_PRESS == aAction)
						state->camControl.actionMoveUp = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.actionMoveUp = false;
				}
				else if (GLFW_KEY_Q == aKey) {
					if (GLFW_PRESS == aAction)
						state->camControl.actionMoveDown = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.actionMoveDown = false;
				}
			}
		}

	}


    void glfw_callback_motion_( GLFWwindow* aWindow, double aMouseXPos, double aMouseYPos )
    {
        if (auto *state = static_cast<State_ *>(glfwGetWindowUserPointer(aWindow)))
        {
            /*  === FPS Style camera controls ===
             *  https://thepentamollisproject.blogspot.com/2018/02/setting-up-first-person-camera-in.html
             * 
             *  === Smooth camera movement ===
             *  https://gamedev.net/forums/topic/624285-smooth-rotation-and-movement-of-camera/4936632/
             */

            if (state->camControl.cameraActive) {

				auto const dx = float(aMouseXPos - fbwidth/2.f) * kMouseSensitivity_;
            	auto const dy = float(aMouseYPos - fbheight/2.f) * kMouseSensitivity_;

                // Update pitch and yaw
                // Multiplying by deltaTime ensures smooth camera movement independent of framerate
                state->camControl.yaw += dx * float(state->deltaTime);
                state->camControl.pitch += dy * float(state->deltaTime);

                // Clamp pitch
                const float maxPitch = std::numbers::pi_v<float> / 2.f;
                if (state->camControl.pitch > maxPitch)
                    state->camControl.pitch = maxPitch;
                else if (state->camControl.pitch < -maxPitch)
                    state->camControl.pitch = -maxPitch;

                // This prevents the mouse moving off-screen
                glfwSetCursorPos( aWindow, fbwidth/2.f, fbheight/2.f );
            }
        }
    }

    void glfw_callback_mouse_( GLFWwindow* aWindow, int aButton, int aAction, int aMods )
    {
		if ( auto* state = static_cast<State_*>(glfwGetWindowUserPointer( aWindow )) )
		{
            if( GLFW_MOUSE_BUTTON_RIGHT == aButton && GLFW_PRESS == aAction ) {

                // Toggle camera control
                state->camControl.cameraActive = !state->camControl.cameraActive;


                // Hide / Show cursor
                if (state->camControl.cameraActive)
                    glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
                else
                    glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }
    }

}

namespace
{
	GLFWCleanupHelper::~GLFWCleanupHelper()
	{
		glfwTerminate();
	}

	GLFWWindowDeleter::~GLFWWindowDeleter()
	{
		if( window )
			glfwDestroyWindow( window );
	}
}
