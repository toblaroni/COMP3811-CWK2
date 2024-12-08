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

#include "../vmlib/vec4.hpp"
#include "../vmlib/mat44.hpp"
#include "../vmlib/mat33.hpp"

#include "defaults.hpp"
#include "loadobj.hpp"

namespace
{
	constexpr char const* kWindowTitle = "COMP3811 - CW2";

    constexpr float kMouseSensitivity_ = 0.1f; // radians per pixel

    float fbwidth, fbheight;
 
    // This will contain the state of our program
    struct State_ {
        ShaderProgram* prog;

        struct CamCtrl_ {
            bool cameraActive;

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

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

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

	glViewport( 0, 0, iwidth, iheight );

	// Load shader program
	ShaderProgram prog( {
		{ GL_VERTEX_SHADER, "assets/cw2/default.vert" },
		{ GL_FRAGMENT_SHADER, "assets/cw2/default.frag" }
	} );

    state.prog = &prog;
    state.camControl.cameraActive = false;
    state.camControl.pitch = 0.f;
    state.camControl.yaw = 0.f;
    state.deltaTime = 0.1f;

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
        Mat44f T = make_translation( Vec3f { 0.f, -3.f, -3.f });

        Mat44f world2camera = Rx * Ry * T;

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
            0,
            1, GL_TRUE, projCameraWorld.v
        );

        glUniformMatrix3fv(
            1,
            1, GL_TRUE, normalMatrix.v
        );

        Vec3f lightDir = normalize( Vec3f{ 0.f, 1.f, -1.f } );
        glUniform3fv( 2, 1, &lightDir.x );

        glUniform3f( 3, 0.9f, 0.9f, 0.6f );
        glUniform3f( 4, 0.05f, 0.05f, 0.05f );

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
                float dx = (float)aMouseXPos - fbwidth/2.f;
                float dy = (float)aMouseYPos - fbheight/2.f;

                // Update pitch and yaw
                // Multiplying by deltaTime ensures smooth camera movement independent of framerate
                state->camControl.yaw += dx * kMouseSensitivity_ * (float)state->deltaTime;
                state->camControl.pitch += dy * kMouseSensitivity_ * (float)state->deltaTime;

                // Clamp pitch
                const float maxPitch = std::numbers::pi_v<float> / 2.0f;
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
