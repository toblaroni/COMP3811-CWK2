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
#include "texture.hpp"
#include "vehicle.hpp"

namespace
{
	constexpr char const* kWindowTitle = "COMP3811 - CW2";

    constexpr float kMovementPerSecond_ = 3.f; // units per second
    constexpr float kMouseSensitivity_ = 0.05f; // radians per pixel

    float fbwidth, fbheight;
 
    // This will contain the state of our program
    struct State_ {
        ShaderProgram* prog;

        /*
         *  === Camera Controls ===
         *  https://learnopengl.com/Getting-started/Camera
         * 
         *  The view matrix in this struct is our camera coordinate system.
         */
        struct CamCtrl_ {
            bool cameraActive;
            bool topDown;

            bool moveFast;
            bool moveSlow;

            bool strafingLeft, strafingRight;
            bool movingForward, movingBackward;
            bool movingUp, movingDown;

            float pitch;    // Looking left / right
            float yaw;      // Looking up / down

            Vec3f cameraPos;
            Vec3f cameraFront;
            Vec3f cameraUp;

            // This contains the coordinate space for the camera
            Mat44f view;
        } camControl;

        double deltaTime;    // This allows smooth camera movement
    };
	
	void glfw_callback_error_( int, char const* );

	void glfw_callback_key_( GLFWwindow*, int, int, int, int );
    void glfw_callback_motion_( GLFWwindow* aWindow, double aMouseXPos, double aMouseYPos );
    void glfw_callback_mouse_( GLFWwindow* aWindow, int aButton, int aAction, int aMods );

    void update_camera_pos( State_* );

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

	#if !defined(__APPLE__)
	// Most platforms will support OpenGL 4.3
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
	#else 
	// Apple has at most OpenGL 4.1, so don't ask for something newer.
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
	#endif // ~__APPLE__

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
	//glEnable( GL_CULL_FACE );
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


    // FIX FOR 4.1
	GLint uProjCameraWorldLocation   = glGetUniformLocation(prog.programId(), "uProjCameraWorld");
	GLint uNormalMatrixLocation      = glGetUniformLocation(prog.programId(), "uNormalMatrix");
	GLint uLightDiffuseLocation      = glGetUniformLocation(prog.programId(), "uLightDiffuse");
	GLint uLightSpecularLocation     = glGetUniformLocation(prog.programId(), "uLightSpecular");
	GLint uSceneAmbientLocation      = glGetUniformLocation(prog.programId(), "uSceneAmbient");
	GLint uViewMatrixLocation        = glGetUniformLocation(prog.programId(), "uViewMatrix");
	GLint uUseTextureLocation        = glGetUniformLocation(prog.programId(), "uUseTexture");
	GLint uLightPosViewSpaceLocation = glGetUniformLocation(prog.programId(), "uLightPosViewSpace");
	GLint uLightDirLocation          = glGetUniformLocation(prog.programId(), "uLightDir");


	// Ensure the locations are valid
	if (uProjCameraWorldLocation == -1 || 
        uNormalMatrixLocation == -1    ||
        uLightDiffuseLocation == -1    ||
        uSceneAmbientLocation == -1    || 
        uViewMatrixLocation == -1      || 
        uLightSpecularLocation == -1   || 
        uLightDirLocation == -1        || 
        uLightPosViewSpaceLocation == -1) {
		std::fprintf(stderr, "Error: Uniform location not found\n");
        // Exit here?
	}


    // Initialise state
    state.prog = &prog;
    state.camControl.cameraActive = false;
    state.camControl.topDown = false;
    state.camControl.pitch = 0.f;
    state.camControl.yaw = std::numbers::pi_v<float> / -2.f;    // Give default of -90 degrees
    state.deltaTime = 0.1f;

    state.camControl.cameraPos = { 0.f, 3.f, 3.f };
    state.camControl.movingBackward = false;
    state.camControl.movingForward = false;
    state.camControl.strafingLeft = false;
    state.camControl.strafingRight = false;
    state.camControl.movingUp = false;
    state.camControl.movingDown = false;

    state.camControl.cameraFront = Vec3f { 0.f, 0.f, -1.f };
    state.camControl.cameraUp = Vec3f{ 0.f, 1.f, 0.f };  // Up vector in coordinate space.
    state.camControl.view = {};

	// Other initialization & loading
	OGL_CHECKPOINT_ALWAYS();

    // Load the terrain and add to VAO
    auto langersoMesh = load_wavefront_obj("assets/cw2/langerso.obj");
    GLuint langersoVao = create_vao(langersoMesh);
    std::size_t langersoVertexCount = langersoMesh.positions.size();

    // Load the texture
    auto textureObjectId = load_texture_2d("assets/cw2/L3211E-4k.jpg");

    // Load the landing pad mesh and create VAO
    auto landingPadMesh = load_wavefront_obj("assets/cw2/landingpad.obj");
    GLuint landingPadVao = create_vao( landingPadMesh );
    std::size_t landingPadVertexCount = landingPadMesh.positions.size();

    // Create Vehicle
    auto vehicle = make_vehicle();
    GLuint vehicleVao = create_vao( vehicle );
    std::size_t vehicleVertexCount = vehicle.positions.size();

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

        // Update camera position
        update_camera_pos( &state );

		//TODO: update state
        Mat44f model2world = kIdentity44f;

        state.camControl.view = look_at(
            state.camControl.cameraPos,
            state.camControl.cameraPos + state.camControl.cameraFront,
            state.camControl.cameraUp
        );

        Mat44f world2camera = state.camControl.view;

        Mat44f projection = make_perspective_projection(
            60.f * std::numbers::pi_v<float> / 180.f,
            fbwidth / float(fbheight),  // Aspect ratio
            0.1f, 100.0f        // Near / far 
        );

        Mat44f projCameraWorld = projection * world2camera * model2world;
        Mat33f normalMatrix = mat44_to_mat33(transpose(invert(model2world)));

        // Translations and projection for first launchpad
        Mat44f model2worldLaunchpad = model2world * make_translation( Vec3f { 3.f, 0.f, -5.f } );
        Mat44f projCameraWorld_LP1 = projection * world2camera * model2worldLaunchpad;
        Mat33f normalMatrix_LP1 = mat44_to_mat33(transpose(invert(model2worldLaunchpad)));

        Mat44f model2worldLaunchpad2 = model2world * make_translation( Vec3f { -7.f, 0.f, 7.f } );
        Mat44f projCameraWorld_LP2 = projection * world2camera * model2worldLaunchpad2;
        Mat33f normalMatrix_LP2 = mat44_to_mat33(transpose(invert(model2worldLaunchpad2)));

        // Space vehicle translations
        Mat44f model2worldVehicle = model2world * make_translation( Vec3f { 3.f, 0.f, -5.f } );
        Mat44f projCameraWorld_V = projection * world2camera * model2worldVehicle;
        Mat33f normalMatrix_V = mat44_to_mat33(transpose(invert(model2worldVehicle)));


		// Draw scene
		OGL_CHECKPOINT_DEBUG();

		//TODO: draw frame
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glUseProgram( prog.programId() );

        Vec3f lightDir = normalize( Vec3f{ 0.f, 1.f, -1.f } );
        glUniform3fv( uLightDirLocation, 1, &lightDir.x );

        // Point light location in world space
        Vec4f lightPos = { 0.f, 5.f, 0.f, 1.f };
        // Transform to camera (view) space 
        Vec4f lightPosViewSpace = world2camera * lightPos;

        glUniform3f( uLightPosViewSpaceLocation,  lightPosViewSpace.x, lightPosViewSpace.y, lightPosViewSpace.z );

        glUniform3f( uLightDiffuseLocation, 1.f, 1.f, 0.f );
        glUniform3f( uLightSpecularLocation, 1.f, 1.f, 1.f );
        glUniform3f( uSceneAmbientLocation, 0.2f, 0.2f, 0.2f );

        glUniformMatrix4fv(
            uViewMatrixLocation,
            1, GL_TRUE, world2camera.v
        );

        glUniformMatrix4fv(
            uProjCameraWorldLocation,
            1, GL_TRUE, projCameraWorld.v
        );

        glUniformMatrix3fv(
            uNormalMatrixLocation,
            1, GL_TRUE, normalMatrix.v
        );

        glUniform1i(uUseTextureLocation, GL_TRUE);

        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, textureObjectId );

        glBindVertexArray( langersoVao );

        glDrawArrays( GL_TRIANGLES, 0, langersoVertexCount );

        glUniform1i(uUseTextureLocation, GL_FALSE);

        // Bind landing pad VAO
        glBindVertexArray( landingPadVao );

        // Draw first landing pad
        glUniformMatrix4fv(
            uProjCameraWorldLocation,
            1, GL_TRUE, projCameraWorld_LP1.v
        );
        glUniformMatrix3fv(
            uNormalMatrixLocation,
            1, GL_TRUE, normalMatrix_LP1.v
        );

        glDrawArrays( GL_TRIANGLES, 0, landingPadVertexCount );


        // Draw second landing pad
        glUniformMatrix4fv(
            uProjCameraWorldLocation,
            1, GL_TRUE, projCameraWorld_LP2.v
        );
        glUniformMatrix3fv(
            uNormalMatrixLocation,
            1, GL_TRUE, normalMatrix_LP2.v
        );
        glDrawArrays( GL_TRIANGLES, 0, landingPadVertexCount );

        // Bind Vehicle VAO
        glBindVertexArray( vehicleVao );

        // Draw first landing pad
        glUniformMatrix4fv(
            uProjCameraWorldLocation,
            1, GL_TRUE, projCameraWorld_V.v
        );
        glUniformMatrix3fv(
            uNormalMatrixLocation,
            1, GL_TRUE, normalMatrix_V.v
        );

        glDrawArrays( GL_TRIANGLES, 0, vehicleVertexCount );

		OGL_CHECKPOINT_DEBUG();

		// Display results
		glfwSwapBuffers( window );
	}

	// Cleanup.
	//TODO: additional cleanup
    glBindVertexArray( 0 );
    glUseProgram( 0 );
    state.prog = nullptr;
	
	return 0;
}
catch( std::exception const& eErr )
{
	std::fprintf( stderr, "Top-level Exception (%s):\n", typeid(eErr).name() );
	std::fprintf( stderr, "%s\n", eErr.what() );
	std::fprintf( stderr, "Bye.\n" );
	return 1;
}

// Helper functions
namespace
{
    void update_camera_pos( State_* state ) {
        float speedModifier = state->camControl.moveFast ? 2.f : state->camControl.moveSlow ? 0.5f : 1.f;
        float velocity = kMovementPerSecond_ * state->deltaTime * speedModifier;

        // Forward / Backward
        if (state->camControl.movingForward)
            state->camControl.cameraPos += velocity * state->camControl.cameraFront;
        if (state->camControl.movingBackward)
            state->camControl.cameraPos -= velocity * state->camControl.cameraFront;

        // Left / Right
        if (state->camControl.strafingLeft)
            // Use cross product to create the 'right vector' then move along that
            state->camControl.cameraPos -= normalize(
                cross(state->camControl.cameraFront, state->camControl.cameraUp)
            ) * velocity;
        if (state->camControl.strafingRight)
            state->camControl.cameraPos += normalize(
                cross(state->camControl.cameraFront, state->camControl.cameraUp)
            ) * velocity;

        // Up / Down
        if (state->camControl.movingUp)
            state->camControl.cameraPos.y -= velocity;
        if (state->camControl.movingDown)
            state->camControl.cameraPos.y += velocity;

        // std::printf("%f, %f\n", state->camControl.cameraPos.x, state->camControl.cameraPos.z);
    }
}


// Callbacks
namespace
{
	void glfw_callback_error_( int aErrNum, char const* aErrDesc )
	{
		std::fprintf( stderr, "GLFW error: %s (%d)\n", aErrDesc, aErrNum );
	}

	void glfw_callback_key_( GLFWwindow* aWindow, int aKey, int, int aAction, int aMod )
	{
		if( GLFW_KEY_ESCAPE == aKey && GLFW_PRESS == aAction )
		{
			glfwSetWindowShouldClose( aWindow, GLFW_TRUE );
			return;
		}

        if (auto *state = static_cast<State_ *>(glfwGetWindowUserPointer(aWindow)))
        {
            if (aAction == GLFW_PRESS && aKey == GLFW_KEY_T) {
                // REMOVE BEFORE SUBMISSION
                // Press T to toggle topdown view
                if (state->camControl.topDown) {
                    state->camControl.cameraPos = { 0.f, 20.f, 0.f };
                    state->camControl.cameraFront = { 0.f, -1.f, 0.f };
                    state->camControl.cameraUp = { 0.f, 0.f, 1.f };
                } else {
                    state->camControl.cameraPos = { 0.f, 3.f, 3.f };
                    state->camControl.cameraFront = Vec3f { 0.f, 0.f, -1.f };
                    state->camControl.cameraUp = Vec3f{ 0.f, 1.f, 0.f };  // Up vector in coordinate space.
                }
                state->camControl.topDown = !state->camControl.topDown;
            }

            if (aAction == GLFW_PRESS || aAction == GLFW_RELEASE) 
            {
                bool isPressed = (aAction == GLFW_PRESS);

                // Move when pressed
                switch (aKey) {
                    case GLFW_KEY_W:
                        state->camControl.movingForward = isPressed; 
                        break;
                    case GLFW_KEY_S:
                        state->camControl.movingBackward = isPressed; 
                        break;
                    case GLFW_KEY_A:
                        state->camControl.strafingLeft = isPressed; 
                        break;
                    case GLFW_KEY_D:
                        state->camControl.strafingRight = isPressed; 
                        break;
                    case GLFW_KEY_E:
                        state->camControl.movingUp = isPressed; 
                        break;
                    case GLFW_KEY_Q:
                        state->camControl.movingDown = isPressed; 
                        break;
                    // Not sure if it's better to use aMod for these?
                    case GLFW_KEY_LEFT_SHIFT:
                        state->camControl.moveFast = isPressed;
                        break;
                    case GLFW_KEY_LEFT_CONTROL:
                        state->camControl.moveSlow = isPressed;
                        break;
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
             * 
             */

            if (state->camControl.cameraActive) {
                float dx = (float)aMouseXPos - fbwidth/2.f;
                float dy = (float)aMouseYPos - fbheight/2.f;

                // Update pitch and yaw
                // Multiplying by deltaTime ensures smooth camera movement independent of framerate
                state->camControl.yaw += dx * kMouseSensitivity_ * (float)state->deltaTime;
                state->camControl.pitch += dy * kMouseSensitivity_ * (float)state->deltaTime;

                // Clamp pitch
                const float maxPitch =  89.f * std::numbers::pi_v<float> / 180.f;
                if (state->camControl.pitch > maxPitch)
                    state->camControl.pitch = maxPitch;
                else if (state->camControl.pitch < -maxPitch)
                    state->camControl.pitch = -maxPitch;


                // This prevents the mouse moving off-screen
                glfwSetCursorPos( aWindow, fbwidth/2.f, fbheight/2.f );

                float cosYaw = std::cos(state->camControl.yaw);
                float sinYaw = std::sin(state->camControl.yaw);
                float cosPitch = std::cos(state->camControl.pitch);
                float sinPitch = std::sin(state->camControl.pitch);

                Vec3f direction = {};
                direction.x = cosYaw * cosPitch;
                direction.y = -sinPitch;
                direction.z = sinYaw * cosPitch;
                state->camControl.cameraFront = normalize(direction);
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
