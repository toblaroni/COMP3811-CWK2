#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <typeinfo>
#include <format>

#include <cstdio>
#include <cstdlib>

#include "../support/error.hpp"
#include "../support/program.hpp"
#include "../support/checkpoint.hpp"
#include "../support/debug_output.hpp"

#include "../vmlib/vec4.hpp"
#include "../vmlib/mat44.hpp"
#include "../vmlib/mat33.hpp"

#include "loadobj.hpp"
#include "texture.hpp"
#include "vehicle.hpp"

#define NUM_LIGHTS 3

namespace
{
	constexpr char const* kWindowTitle = "COMP3811 - CW2";

    constexpr float kMovementPerSecond_ = 3.f; // units per second
    constexpr float kMouseSensitivity_ = 0.05f; // radians per pixel

    int fbwidth = 0;
    int fbheight = 0;

    struct Light {
        Vec3f position;     // Make sure to convert to camera space
        Vec3f diffuse;
        Vec3f specular;
        Vec3f ambient;
		Vec3f offset;		// This is lights offset from the ship
    };
 
    // This will contain the state of our program
    struct State_ {
        ShaderProgram* prog;
		ShaderProgram* UI_prog;
        
        bool isSplitScreen = false;
        
        /*
         *  === Camera Controls ===
         *  https://learnopengl.com/Getting-started/Camera
         * 
         *  The view matrix in this struct is our camera coordinate system.
         */
        struct CamCtrl_ {
            bool cameraActive = false;
            bool topDown = false;

            int8_t camView = 0;

            bool moveFast = false;
            bool moveSlow = false;

            bool strafingLeft = false, strafingRight = false;
            bool movingForward = false, movingBackward = false;
            bool movingUp = false, movingDown = false;

            float pitch = 0.f;  // Looking left / right
            float yaw = std::numbers::pi_v<float> / -2.f;      // Looking up / down

            Vec3f cameraPos = { 0.f, 3.f, 3.f };
            Vec3f cameraFront = { 0.f, 0.f, -1.f };
            Vec3f cameraUp { 0.f, 1.f, 0.f };

            // This contains the coordinate space for the camera
            Mat44f view = {};

        } camControl;

        double deltaTime;    // This allows smooth camera movement


        // This will hold all data required for rendering 
        struct RenderData_ {
            GLuint langersoVertexCount;
            GLuint landingPadVertexCount;
            GLuint vehicleVertexCount;

            // Uniform locations
            GLuint uDirectLightDirLocation;
            GLuint uDirectLightDiffuseLocation;
            GLuint uDirectLightAmbientLocation;

            GLuint uLightPosLocations[NUM_LIGHTS] = {};
            GLuint uLightDiffuseLocations[NUM_LIGHTS] = {};
            GLuint uLightSpecularLocations[NUM_LIGHTS] = {};
            GLuint uSceneAmbientLocations[NUM_LIGHTS] = {};

            GLuint uProjCameraWorldLocation;
            GLuint uUseTextureLocation;
            GLuint uNormalMatrixLocation;
            GLuint uViewMatrixLocation;
            GLuint uModel2WorldLocation;

            // Matrices
            Mat44f world2camera;
            Mat44f projection;

            // Point Lights
            std::vector<Light> lights = {};
			std::vector<Vec3f> lightOrigins = {};

            // VAO's
            GLuint langersoVao;
            GLuint landingPadVao;
            GLuint vehicleVao;

            // Texture ID
            GLuint textureObjectId;

        } renderData;


        struct VehicleCtrl_ {
            bool launch = false;
            Vec3f origin = { 3.f, 0.f, -5.f };
            Vec3f position = origin;

            float time = 0.f;
            float theta = 0.f;
        } vehicleControl;

    };
	
	void glfw_callback_error_( int, char const* );

	void glfw_callback_key_( GLFWwindow*, int, int, int, int );
    void glfw_callback_motion_( GLFWwindow* aWindow, double aMouseXPos, double aMouseYPos );
    void glfw_callback_mouse_( GLFWwindow* aWindow, int aButton, int aAction, int aMods );

    void update_camera_pos( State_* );
    void renderScene( State_& );
	void initialisePointLights( State_& );

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

	float wscale = 1.f, hscale = 1.f;

	#if defined(__APPLE__)
		glfwGetWindowContentScale( window, &wscale, &hscale);
	#endif

	iwidth = int(iwidth / wscale);
	iheight = int(iheight / hscale);

	glViewport( 0, 0, iwidth, iheight );

	// Load shader program
	ShaderProgram prog( {
		{ GL_VERTEX_SHADER, "assets/cw2/default.vert" },
		{ GL_FRAGMENT_SHADER, "assets/cw2/default.frag" }
	} );

	// Load UI shader program
	ShaderProgram UI_prog( {
		{ GL_VERTEX_SHADER, "assets/cw2/UI.vert" },
		{ GL_FRAGMENT_SHADER, "assets/cw2/UI.frag" }
	} );


    // FIX FOR 4.1
    state.renderData.uDirectLightDirLocation     = glGetUniformLocation(prog.programId(), "uDirectLightDir");
	state.renderData.uDirectLightAmbientLocation = glGetUniformLocation(prog.programId(), "uDirectLightAmbient");
	state.renderData.uDirectLightDiffuseLocation = glGetUniformLocation(prog.programId(), "uDirectLightDiffuse");

	state.renderData.uProjCameraWorldLocation = glGetUniformLocation(prog.programId(), "uProjCameraWorld");
	state.renderData.uNormalMatrixLocation    = glGetUniformLocation(prog.programId(), "uNormalMatrix");
	state.renderData.uViewMatrixLocation      = glGetUniformLocation(prog.programId(), "uViewMatrix");
	state.renderData.uUseTextureLocation      = glGetUniformLocation(prog.programId(), "uUseTexture");
	state.renderData.uModel2WorldLocation     = glGetUniformLocation(prog.programId(), "uModel2World");

    // Generate locations for lights
    for (int i = 0; i < NUM_LIGHTS; ++i) {
		// Light Position
		std::string lightPosUniform = std::format("uLightPos[{}]", i);
		state.renderData.uLightPosLocations[i] = glGetUniformLocation(prog.programId(), lightPosUniform.c_str());

		// Light Diffuse
		std::string lightDiffuseUniform = std::format("uLightDiffuse[{}]", i);
		state.renderData.uLightDiffuseLocations[i] = glGetUniformLocation(prog.programId(), lightDiffuseUniform.c_str());

		// Light Specular
		std::string lightSpecularUniform = std::format("uLightSpecular[{}]", i);
		state.renderData.uLightSpecularLocations[i] = glGetUniformLocation(prog.programId(), lightSpecularUniform.c_str());

		// Scene Ambient
		std::string sceneAmbientUniform = std::format("uSceneAmbient[{}]", i);
		state.renderData.uSceneAmbientLocations[i] = glGetUniformLocation(prog.programId(), sceneAmbientUniform.c_str());

		// Check Uniform Locations
		if (state.renderData.uLightPosLocations[i] == static_cast<GLuint>(-1)) {
			fprintf(stderr, "Error: Uniform location for %s not found!\n", lightPosUniform.c_str());
		}
		if (state.renderData.uLightDiffuseLocations[i] == static_cast<GLuint>(-1)) {
			fprintf(stderr, "Error: Uniform location for %s not found!\n", lightDiffuseUniform.c_str());
		}
		if (state.renderData.uLightSpecularLocations[i] == static_cast<GLuint>(-1)) {
			fprintf(stderr, "Error: Uniform location for %s not found!\n", lightSpecularUniform.c_str());
		}
		if (state.renderData.uSceneAmbientLocations[i] == static_cast<GLuint>(-1)) {
			fprintf(stderr, "Error: Uniform location for %s not found!\n", sceneAmbientUniform.c_str());
		}
	}



	// Ensure the locations are valid
	if (state.renderData.uProjCameraWorldLocation == static_cast<GLuint>(-1) || 
        state.renderData.uNormalMatrixLocation == static_cast<GLuint>(-1)    ||
        state.renderData.uViewMatrixLocation == static_cast<GLuint>(-1)      || 
        state.renderData.uDirectLightDirLocation == static_cast<GLuint>(-1)  || 
        state.renderData.uModel2WorldLocation == static_cast<GLuint>(-1)     || 
        state.renderData.uUseTextureLocation == static_cast<GLuint>(-1)) {
		std::fprintf(stderr, "Error: Uniform location not found\n");
	}


    // Initialise state
    state.prog = &prog;
	state.UI_prog = &UI_prog;

    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	// Other initialization & loading
	OGL_CHECKPOINT_ALWAYS();

    // Load the terrain and add to VAO
    auto langersoMesh = load_wavefront_obj("assets/cw2/langerso.obj");
    state.renderData.langersoVao = create_vao(langersoMesh);
    state.renderData.langersoVertexCount = langersoMesh.positions.size();

    // Load the texture
    state.renderData.textureObjectId = load_texture_2d("assets/cw2/L3211E-4k.jpg");

    // Load the landing pad mesh and create VAO
    auto landingPadMesh = load_wavefront_obj("assets/cw2/landingpad.obj");
    state.renderData.landingPadVao = create_vao( landingPadMesh );
    state.renderData.landingPadVertexCount = landingPadMesh.positions.size();

    // Create Vehicle
    auto vehicle = make_vehicle();
    state.renderData.vehicleVao = create_vao( vehicle );
    state.renderData.vehicleVertexCount = vehicle.positions.size();

	initialisePointLights( state );

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

			fbwidth = nwidth;
			fbheight = nheight;

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

        if (state.camControl.camView == 1) {
            state.camControl.cameraPos = state.vehicleControl.position + Vec3f{ 1.f, 3.f, -3.f };
            state.camControl.cameraFront = normalize(state.vehicleControl.position - state.camControl.cameraPos);
            state.camControl.cameraUp = { 0.f, 1.f, 0.f };
        }
        else if (state.camControl.camView == 2) {
            state.camControl.cameraPos = Vec3f{ 0.f, 0.5f, 0.f };
            state.camControl.cameraFront = normalize(state.vehicleControl.position - state.camControl.cameraPos);
            state.camControl.cameraUp = { 0.f, 1.f, 0.f };
        }

        // Update camera position
        update_camera_pos( &state );

		//TODO: update state

        state.camControl.view = look_at(
            state.camControl.cameraPos,
            state.camControl.cameraPos + state.camControl.cameraFront,  // This is the target AKA what we want to look at
            state.camControl.cameraUp
        );

        state.renderData.world2camera = state.camControl.view;

        state.renderData.projection = make_perspective_projection(
            60.f * std::numbers::pi_v<float> / 180.f,
            fbwidth / float(fbheight),  // Aspect ratio
            0.1f, 100.0f                // Near / far 
        );


		// Draw scene
		OGL_CHECKPOINT_DEBUG();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(prog.programId());

		//TODO: draw frame
        if (!state.isSplitScreen) {
            state.renderData.projection = make_perspective_projection(
                60.f * std::numbers::pi_v<float> / 180.f,
                fbwidth / float(fbheight),                  // Aspect ratio
                0.1f, 100.0f                                // Near / far 
            );


            glUniformMatrix4fv(
                state.renderData.uViewMatrixLocation, 1,
                GL_TRUE, state.renderData.world2camera.v
            );

            renderScene(state);
        } else {
            state.renderData.projection = make_perspective_projection(
                60.f * std::numbers::pi_v<float> / 180.f,
                fbwidth/2.f / float(fbheight),  // Aspect ratio
                0.1f, 100.0f                // Near / far 
            );

            glViewport(0, 0, fbwidth/2, fbheight);

            glUniformMatrix4fv(state.renderData.uViewMatrixLocation, 1,
                                GL_TRUE, state.renderData.world2camera.v);

            renderScene(state);

            glViewport(fbwidth/2, 0, fbwidth/2, fbheight);

            renderScene(state);
        }

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
	void initialisePointLights( State_& state ) {

		Vec3f lightPos1 = Vec3f{ 2.9f,  0.3f, -4.75f };
		Vec3f lightPos2 = Vec3f{ 2.7f,  0.3f, -5.f };
		Vec3f lightPos3 = Vec3f{ 2.99f, 0.3f, -5.26f };

		// Create lights
		state.renderData.lights = {
			Light {
				lightPos1,  
				Vec3f{1.0f, 1.0f, 1.0f},
				Vec3f{1.5f, 1.5f, 1.5f},
				Vec3f{0.3f, 0.3f, 0.3f},
				lightPos1 - state.vehicleControl.origin		// Offset from the ship
			},
			Light {
				lightPos2,
				Vec3f{0.0f, 1.0f, 0.0f}, 
				Vec3f{1.5f, 1.0f, 0.5f}, 
				Vec3f{0.2f, 0.2f, 0.2f},  
				lightPos2 - state.vehicleControl.origin
			},
			Light {
				lightPos3,
				Vec3f{0.0f, 0.0f, 1.0f}, 
				Vec3f{1.5f, 0.5f, 1.5f}, 
				Vec3f{0.2f, 0.2f, 0.3f},
				lightPos3 - state.vehicleControl.origin
			}
		};
	}
	
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

        // std::printf("%f, %f, %f\n", state->camControl.cameraPos.x, state->camControl.cameraPos.y, state->camControl.cameraPos.z);
    }

    void drawMesh(
        GLuint vao, 
        GLuint vertexCount, 
        const Mat44f &projCameraWorld, 
        const Mat33f &normalMatrix, 
        State_ &state
    ) {
        glUniformMatrix4fv(state.renderData.uProjCameraWorldLocation, 1, GL_TRUE, projCameraWorld.v);
        glUniformMatrix3fv(state.renderData.uNormalMatrixLocation, 1, GL_TRUE, normalMatrix.v);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }

    // Contains main rendering logic
    void renderScene( State_ &state ) {

		glUseProgram( state.prog->programId() );
		glEnable( GL_DEPTH_TEST );


        // === Setup Lighting ===
        // Original directional lighting
        Vec3f directLightDir = normalize( Vec3f{ 0.f, 1.f, -1.f } );

        glUniform3fv( state.renderData.uDirectLightDirLocation, 1, &directLightDir.x );
        glUniform3f( state.renderData.uDirectLightDiffuseLocation, 0.5f, 0.5f, 0.0f );
        glUniform3f( state.renderData.uDirectLightAmbientLocation, 0.1f, 0.1f, 0.1f );

        // Space vehicle translations
        if ( state.vehicleControl.launch ) {
                
            // Define constants
            float accelerationUp = 2.0f; // Upward acceleration during thrust
            float gravity = 2.5f;        // Gravitational acceleration
            float accelerationZ = 1.5f;  // Forward acceleration in Z
            float maxThrustTime = 2.0f;   // Time for upward thrust phase
            state.vehicleControl.time += state.deltaTime;

            // Compute time into deceleration phase
            float decelTime = std::max(0.0f, state.vehicleControl.time - maxThrustTime);

            // Calculate velocity in the y-axis
            float velocityY = 0.0f;

            if (state.vehicleControl.time <= maxThrustTime) {
                // During thrust phase
                velocityY = accelerationUp * state.vehicleControl.time;
            } 
            else {
                // During deceleration phase under gravity
                velocityY = accelerationUp * maxThrustTime - gravity * decelTime;
                if (velocityY <= 0.0f) {
                    // Stop vertical motion when velocity reaches zero
                    velocityY = 0.0f;
                }
            }

            state.vehicleControl.position.y += velocityY * state.deltaTime;

            // Update velocity in Z (starts at 0 and accelerates)
            float velocityZ = accelerationZ * state.vehicleControl.time;

            // Update position in the z-axis
            state.vehicleControl.position.z += velocityZ * state.deltaTime;

			// Update lights to follow the ship
			for (int i = 0; i < NUM_LIGHTS; ++i)
			{
				// Define a fixed offset for each light relative to the ship's position
				Vec3f lightOffset = state.renderData.lights[i].offset;

				// Calculate rotated offset based on ship's orientation
				float cosTheta = std::cos(state.vehicleControl.theta);
				float sinTheta = std::sin(state.vehicleControl.theta);
				Vec3f rotatedOffset = {
					cosTheta * lightOffset.x - sinTheta * lightOffset.z,
					lightOffset.y, // Y remains unchanged for rotation around Z
					sinTheta * lightOffset.x + cosTheta * lightOffset.z
				};

				// Update light position to follow the ship
				state.renderData.lights[i].position = state.vehicleControl.position + rotatedOffset;
			}

			// Compute rotation angle based on velocity vector
            state.vehicleControl.theta = std::atan2(velocityZ, velocityY);
            
        }


        // Point lights
		// Do we need to do this every frame?
        for (int i = 0; i < NUM_LIGHTS; ++i) {
            glUniform3fv( state.renderData.uLightPosLocations[i], 1, &state.renderData.lights[i].position.x );
            glUniform3fv( state.renderData.uLightDiffuseLocations[i], 1, &state.renderData.lights[i].diffuse.x );
            glUniform3fv( state.renderData.uLightSpecularLocations[i], 1, &state.renderData.lights[i].specular.x );
            glUniform3fv( state.renderData.uSceneAmbientLocations[i], 1, &state.renderData.lights[i].ambient.x );
        }

        // === Setting up models ===
        // Langerso translations
        Mat44f model2world = kIdentity44f;
        Mat44f projCameraWorld = state.renderData.projection * state.renderData.world2camera * model2world;
        Mat33f normalMatrix = mat44_to_mat33(transpose(invert(model2world)));

        // Translations and projection for first launchpad
        Mat44f model2worldLaunchpad =  make_translation( Vec3f { 3.f, 0.f, -5.f } ) * model2world;
        Mat44f projCameraWorld_LP1 = state.renderData.projection * state.renderData.world2camera * model2worldLaunchpad;
        Mat33f normalMatrix_LP1 = mat44_to_mat33(transpose(invert(model2worldLaunchpad)));

        Mat44f model2worldLaunchpad2 = make_translation( Vec3f { -7.f, 0.f, 7.f } ) * model2world;
        Mat44f projCameraWorld_LP2 = state.renderData.projection * state.renderData.world2camera * model2worldLaunchpad2;
        Mat33f normalMatrix_LP2 = mat44_to_mat33(transpose(invert(model2worldLaunchpad2)));

        // Combine translation and rotation
        Mat44f model2worldVehicle = make_translation(state.vehicleControl.position) * make_rotation_x(state.vehicleControl.theta);
        Mat44f projCameraWorld_V = state.renderData.projection * state.renderData.world2camera * model2worldVehicle;
        Mat33f normalMatrix_V = mat44_to_mat33(transpose(invert(model2worldVehicle)));

        // === Drawing ===
        // Langerso mesh
        glUniformMatrix4fv(
            state.renderData.uModel2WorldLocation, 1,
            GL_TRUE, model2world.v
        );
        glUniform1i(state.renderData.uUseTextureLocation, GL_TRUE);
        drawMesh(state.renderData.langersoVao, state.renderData.langersoVertexCount, projCameraWorld, normalMatrix, state);

        // Draw Vehicle
        glUniform1i(state.renderData.uUseTextureLocation, GL_FALSE);

        glUniformMatrix4fv(
            state.renderData.uModel2WorldLocation, 1,
            GL_TRUE, model2worldVehicle.v
        );

        drawMesh(state.renderData.vehicleVao, state.renderData.vehicleVertexCount, projCameraWorld_V, normalMatrix_V, state);

        // Draw first launch pad
        glUniformMatrix4fv(
            state.renderData.uModel2WorldLocation, 1,
            GL_TRUE, model2worldLaunchpad.v
        );

        drawMesh(state.renderData.landingPadVao, state.renderData.landingPadVertexCount, projCameraWorld_LP1, normalMatrix_LP1, state);

        glUniformMatrix4fv(
            state.renderData.uModel2WorldLocation, 1,
            GL_TRUE, model2worldLaunchpad2.v
        );

        // Draw second launch pad
        drawMesh(state.renderData.landingPadVao, state.renderData.landingPadVertexCount, projCameraWorld_LP2, normalMatrix_LP2, state);


		// === UI ===
        glUseProgram( state.UI_prog->programId() );
		glDisable( GL_DEPTH_TEST );

		// Here we define the data
		static float const kPositions[] = {
			// Original triangle
			0.f, 0.8f,
			-0.7f, -0.8f,
			0.7f, -0.8f,
		};

		static float const kColors[] = {
			0.5f, 0.5f, 0.5f, 0.5f,
			0.5f, 0.5f, 0.5f, 0.5f,
			0.5f, 0.5f, 0.5f, 0.5f,
		};

		GLuint positionVBO = 0;
		glGenBuffers( 1, &positionVBO );
		glBindBuffer( GL_ARRAY_BUFFER, positionVBO );
		glBufferData( GL_ARRAY_BUFFER, sizeof(kPositions), kPositions, GL_STATIC_DRAW );

		GLuint colorVBO = 0;
		glGenBuffers( 1, &colorVBO );
		glBindBuffer( GL_ARRAY_BUFFER, colorVBO );
		glBufferData( GL_ARRAY_BUFFER, sizeof(kColors), kColors, GL_STATIC_DRAW );

		
		GLuint vao = 0;
		glGenVertexArrays( 1, &vao );
		glBindVertexArray( vao );



		glBindBuffer( GL_ARRAY_BUFFER, positionVBO ); 
		glVertexAttribPointer(
			0,
			2, GL_FLOAT, GL_FALSE,
			0,
			0
		);
		glEnableVertexAttribArray( 0 );

		glBindBuffer( GL_ARRAY_BUFFER, colorVBO ); 
		glVertexAttribPointer(
			1,
			4, GL_FLOAT, GL_FALSE,
			0,
			0
		);
		glEnableVertexAttribArray( 1 );

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


		glDrawArrays( GL_TRIANGLES, 0, 3 );

		// Cleanup
		glBindVertexArray( 0 );
		glBindBuffer( GL_ARRAY_BUFFER, 0 );

		glDeleteBuffers( 1, &colorVBO );
		glDeleteBuffers( 1, &positionVBO );

		glDisable(GL_BLEND);

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

            if (aAction == GLFW_PRESS && aKey == GLFW_KEY_C) {

                if (state->camControl.camView == 0) {
                    state->camControl.camView = 1;
                }
                else if (state->camControl.camView == 1) {
                    state->camControl.camView = 2;
                }
                else {
                    state->camControl.camView = 0;
                }

            }

            if (aAction == GLFW_PRESS && aKey == GLFW_KEY_F) {
                state->vehicleControl.launch ^= true; 
            }

            if (aAction == GLFW_PRESS && aKey == GLFW_KEY_R) {
                state->vehicleControl.launch = false;
                state->vehicleControl.origin = { 3.f, 0.f, -5.f };
                state->vehicleControl.position = state->vehicleControl.origin;
                state->vehicleControl.time = 0.f;
                state->vehicleControl.theta = 0.f;

				// Reset lights
				initialisePointLights( *state );
            }

            if (aAction == GLFW_PRESS && aKey == GLFW_KEY_V) { state->isSplitScreen = !state->isSplitScreen; }

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
                    case GLFW_KEY_RIGHT_SHIFT:
                        state->camControl.moveFast = isPressed;
                        break;
                    case GLFW_KEY_LEFT_CONTROL: 
                    case GLFW_KEY_RIGHT_CONTROL:
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
