#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <typeinfo>
#include <cstdio>
#include <cstdlib>

#include "../support/error.hpp"
#include "../support/program.hpp"
#include "../support/checkpoint.hpp"
#include "../support/debug_output.hpp"

#include "../vmlib/mat44.hpp"
#include "../vmlib/mat33.hpp"

#include "user_interface.hpp"
#include "loadobj.hpp"
#include "texture.hpp"
#include "vehicle.hpp"
#include "particle.hpp"

#include <fontstash.h>
#include <stb_truetype.h>
#include <chrono>

#define FONTSTASH_IMPLEMENTATION

#define NUM_LIGHTS 3

// Camera Views
#define FREE_ROAM 0
#define FIXED_DISTANCE 1
#define GROUND_POSITION 2

// UI Button States
#define NORMAL 0
#define MOUSE_OVER 1
#define PRESSED 2

//#define ENABLE_TIMING

namespace
{
    constexpr char const* kWindowTitle = "COMP3811 - CW2";

    constexpr float kMovementPerSecond_ = 3.f; // units per second
    constexpr float kMouseSensitivity_ = 0.05f; // radians per pixel

    int fbwidth = 0;
    int fbheight = 0;

    UserInterface UI;

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

        double deltaTime;

        bool isSplitScreen = false;

        ParticleSystem *particleSystem;
        VehicleCtrl_ vehicleControl;

        /*
        *  === Camera Controls ===
        *  https://learnopengl.com/Getting-started/Camera
        *
        *  The view matrix in this struct is our camera coordinate system.
        *  This can be heavily refactored since both cameras share a lot of variables.
        *  AKA Most of it should be global.
        *
        *  TODO: ADD CAMERA STUFF TO IT"S OWN HEADER FILE
        */
        struct CamCtrl_ {

            size_t camView = FREE_ROAM;

            float pitch = 0.f;  // Looking left / right
            float yaw = std::numbers::pi_v<float> / -2.f;      // Looking up / down

            Vec3f cameraPos = { 0.f, 3.f, 3.f };
            Vec3f cameraFront = { 0.f, 0.f, -1.f };
            Vec3f cameraUp { 0.f, 1.f, 0.f };

            // This contains the coordinate space for the camera
            Mat44f view = {};

            Mat44f getView() {
                return look_at(
                    cameraPos,
                    cameraPos + cameraFront,  // This is the target AKA what we want to look at
                    cameraUp
                );
            };

        };

        CamCtrl_ camControl;
        CamCtrl_ camControl2;   // Second camera for split screen

        struct FreeRoamCam {
            bool cameraActive = false;

            // These are all for the global 'free roam' camera
            Vec3f cameraPos = { 0.f, 3.f, 3.f };
            Vec3f cameraFront = { 0.f, 0.f, -1.f };
            Vec3f cameraUp = { 0.f, 1.f, 0.f };

            bool moveFast = false;
            bool moveSlow = false;

            bool strafingLeft = false, strafingRight = false;
            bool movingForward = false, movingBackward = false;
            bool movingUp = false, movingDown = false;

            float pitch = 0.f;  // Looking left / right
            float yaw = std::numbers::pi_v<float> / -2.f;      // Looking up / down
        } freeRoamCtrls;


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
            GLuint uModel2WorldLocation;

            GLuint uButtonActiveColorLocation;
            GLuint uButtonOutlineLocation;

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

            GLuint UI_vao;

            // Texture ID
            GLuint textureObjectId;

        } renderData;

        #ifdef ENABLE_TIMING
        GLuint queries[12];
        size_t qCount = 0;

        std::chrono::_V2::system_clock::time_point startF2F;
        #endif



    };

    void glfw_callback_error_( int, char const* );

    void glfw_callback_key_( GLFWwindow*, int, int, int, int );
    void glfw_callback_motion_( GLFWwindow* aWindow, double aMouseXPos, double aMouseYPos );
    void glfw_callback_mouse_( GLFWwindow* aWindow, int aButton, int aAction, int aMods );

    // Forward declarations
    void update_camera_pos( State_& );
    void renderScene( State_& );
    void initialisePointLights( State_& );
    void configureCamera( State_& );

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
    #	else
    // Apple has at most OpenGL 4.1, so don't ask for something newer.
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
    #	endif // ~__APPLE__

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

    // Load particles shader program
    ShaderProgram particle_prog( {
        { GL_VERTEX_SHADER, "assets/cw2/particle.vert" },
        { GL_FRAGMENT_SHADER, "assets/cw2/particle.frag" }
    } );


    UI.add_button("Launch", { -0.5f, -0.6f }, { -0.1f, -1.f }, { 0.5f, 0.5f, 0.5f, 1.f });
    UI.add_button("Reset", { 0.1f, -0.6f }, { 0.5f, -1.f }, { 0.5f, 0.5f, 0.5f, 1.f });


    // FIX FOR 4.1
    state.renderData.uDirectLightDirLocation     = glGetUniformLocation(prog.programId(), "uDirectLightDir");
    state.renderData.uDirectLightAmbientLocation = glGetUniformLocation(prog.programId(), "uDirectLightAmbient");
    state.renderData.uDirectLightDiffuseLocation = glGetUniformLocation(prog.programId(), "uDirectLightDiffuse");

    state.renderData.uProjCameraWorldLocation = glGetUniformLocation(prog.programId(), "uProjCameraWorld");
    state.renderData.uNormalMatrixLocation    = glGetUniformLocation(prog.programId(), "uNormalMatrix");
    state.renderData.uUseTextureLocation      = glGetUniformLocation(prog.programId(), "uUseTexture");
    state.renderData.uModel2WorldLocation     = glGetUniformLocation(prog.programId(), "uModel2World");

    GLuint uWorldCameraPosLocation = glGetUniformLocation(prog.programId(), "uWorldCameraPos");

    state.renderData.uButtonActiveColorLocation  = glGetUniformLocation(UI_prog.programId(), "uButtonActiveColor");
    state.renderData.uButtonOutlineLocation  = glGetUniformLocation(UI_prog.programId(), "uButtonOutline");

    // Generate locations for lights
    for (int i = 0; i < NUM_LIGHTS; ++i) {
        // Light Position
        std::string lightPosUniform = "uLightPos[" + std::to_string(i) + "]";
        state.renderData.uLightPosLocations[i] = glGetUniformLocation(prog.programId(), lightPosUniform.c_str());

        // Light Diffuse
        std::string lightDiffuseUniform = "uLightDiffuse[" + std::to_string(i) + "]";
        state.renderData.uLightDiffuseLocations[i] = glGetUniformLocation(prog.programId(), lightDiffuseUniform.c_str());

        // Light Specular
        std::string lightSpecularUniform = "uLightSpecular[" + std::to_string(i) + "]";
        state.renderData.uLightSpecularLocations[i] = glGetUniformLocation(prog.programId(), lightSpecularUniform.c_str());

        // Scene Ambient
        std::string sceneAmbientUniform = "uSceneAmbient[" + std::to_string(i) + "]";
        state.renderData.uSceneAmbientLocations[i] = glGetUniformLocation(prog.programId(), sceneAmbientUniform.c_str());
        
        // Check Uniform Locations
        if (state.renderData.uLightPosLocations[i] == static_cast<GLuint>(-1))
            fprintf(stderr, "Error: Uniform location for %s not found!\n", lightPosUniform.c_str());
        if (state.renderData.uLightDiffuseLocations[i] == static_cast<GLuint>(-1))
            fprintf(stderr, "Error: Uniform location for %s not found!\n", lightDiffuseUniform.c_str());
        if (state.renderData.uLightSpecularLocations[i] == static_cast<GLuint>(-1))
            fprintf(stderr, "Error: Uniform location for %s not found!\n", lightSpecularUniform.c_str());
        if (state.renderData.uSceneAmbientLocations[i] == static_cast<GLuint>(-1))
            fprintf(stderr, "Error: Uniform location for %s not found!\n", sceneAmbientUniform.c_str());
    }

    // Ensure the locations are valid
    if ( 
        state.renderData.uProjCameraWorldLocation == static_cast<GLuint>(-1) ||
         state.renderData.uNormalMatrixLocation == static_cast<GLuint>(-1)    ||
         state.renderData.uDirectLightDirLocation == static_cast<GLuint>(-1)  ||
         state.renderData.uModel2WorldLocation == static_cast<GLuint>(-1)     ||
         uWorldCameraPosLocation == static_cast<GLuint>(-1)                   ||
         state.renderData.uUseTextureLocation == static_cast<GLuint>(-1) 
    ) {
        std::fprintf(stderr, "Error: Uniform location not found\n");
    }



    // Assign shader programs
    state.prog = &prog;
	state.UI_prog = &UI_prog;

    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

    // Init particle system
    GLuint particleSpriteId = load_texture_2d("assets/cw2/particle.png");
    state.particleSystem = new ParticleSystem( particle_prog, particleSpriteId, 50 );

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

    state.renderData.UI_vao = create_UI_vao(UI);

    initialisePointLights( state );

    double last = glfwGetTime();

    // Main loop
    while( !glfwWindowShouldClose( window ) )
    {

        #ifdef ENABLE_TIMING
		glGenQueries(12, state.queries);

        state.startF2F = std::chrono::high_resolution_clock::now();
        #endif



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
        // TODO: CREATE UPDATE FUNCTION?
        double currentTime = glfwGetTime();
        state.deltaTime = currentTime - last;
        last = currentTime;

        if (state.vehicleControl.launch) {
            state.particleSystem->update(
                state.deltaTime,
                state.vehicleControl.position + Vec3f { 0.f, 0.1f, 0.f },
                state.vehicleControl.velocity,
                8,
                state.camControl.cameraPos
            );
        }

        OGL_CHECKPOINT_DEBUG();

        // Draw scene

        glUseProgram(prog.programId());

        glEnable( GL_DEPTH_TEST );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDepthFunc(GL_LESS); // Ensure closer fragments overwrite farther ones

        // === Setup Lighting ===
        // Original directional lighting
        Vec3f directLightDir = normalize( Vec3f{ 0.f, 1.f, -1.f } );

        glUniform3fv( state.renderData.uDirectLightDirLocation, 1, &directLightDir.x );
        glUniform3f( state.renderData.uDirectLightDiffuseLocation, 0.8f, 0.8f, 0.3f );
        glUniform3f( state.renderData.uDirectLightAmbientLocation, 0.1f, 0.1f, 0.1f );

        // === Update Vehicle ===
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

            state.vehicleControl.velocity = { 0.f, velocityY, velocityZ };
        }

        // Point lights
        for (int i = 0; i < NUM_LIGHTS; ++i) {
            glUniform3fv( state.renderData.uLightPosLocations[i], 1, &state.renderData.lights[i].position.x );
            glUniform3fv( state.renderData.uLightDiffuseLocations[i], 1, &state.renderData.lights[i].diffuse.x );
            glUniform3fv( state.renderData.uLightSpecularLocations[i], 1, &state.renderData.lights[i].specular.x );
            glUniform3fv( state.renderData.uSceneAmbientLocations[i], 1, &state.renderData.lights[i].ambient.x );
        }

        configureCamera( state );

        if (!state.isSplitScreen) {

            glViewport(0, 0, fbwidth, fbheight);

            state.renderData.world2camera = state.camControl.getView();

            state.renderData.projection = make_perspective_projection(
                60.f * std::numbers::pi_v<float> / 180.f,
                fbwidth / float(fbheight),                  // Aspect ratio
                0.1f, 100.0f                                // Near / far
            );

            glUniform3fv( uWorldCameraPosLocation, 1, &state.camControl.cameraPos.x );

            renderScene( state );

        } else {

            state.renderData.world2camera = state.camControl.getView();

            state.renderData.projection = make_perspective_projection(
                60.f * std::numbers::pi_v<float> / 180.f,
                fbwidth/2.f / float(fbheight),              // Aspect ratio
                0.1f, 100.0f                                // Near / far
            );

            glViewport(0, 0, fbwidth/2, fbheight);
            renderScene( state );

            // === Right hand side ===
            state.renderData.world2camera = state.camControl2.getView();
            glViewport(fbwidth/2, 0, fbwidth/2, fbheight);
            renderScene( state );
        }


        if (state.vehicleControl.hasLaunched) {
            if ( !state.isSplitScreen ) {
                state.particleSystem->draw(
                    state.renderData.projection * state.renderData.world2camera,
                    state.renderData.world2camera
                );
            } else {
                glViewport(0, 0, fbwidth/2, fbheight);
                state.particleSystem->draw(
                    state.renderData.projection * state.camControl.getView(),
                    state.camControl.getView()
                );

                glViewport(fbwidth/2, 0, fbwidth/2, fbheight);
                state.particleSystem->draw(
                    state.renderData.projection * state.camControl2.getView(),
                    state.camControl2.getView()
                );
            }
        }

        // === UI ===
        glViewport( 0, 0, fbwidth, fbheight );

        glUseProgram( state.UI_prog->programId() );
        glDisable( GL_DEPTH_TEST );

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable( GL_PROGRAM_POINT_SIZE );


        glBindVertexArray(state.renderData.UI_vao);

        for (size_t i = 0; i < UI.buttons.size(); i++) {
            if (UI.buttons[i].state == MOUSE_OVER) {
                // If Hover over
                static float const baseColor[] = {1.f, 1.f, 1.f, 1.f};
                glUniform4fv(state.renderData.uButtonActiveColorLocation, 1, baseColor);
            }
            else if (UI.buttons[i].state == PRESSED) {
                // If pressed
                static float const baseColor[] = {0.25f, 0.25f, 0.25f, 1.f};
                glUniform4fv(state.renderData.uButtonActiveColorLocation, 1, baseColor);
            }
            else {
                // If inactive
                static float const baseColor[] = {1.f, 1.f, 1.f, 0.5f};
                glUniform4fv(state.renderData.uButtonActiveColorLocation, 1, baseColor);
            }

            #ifdef ENABLE_TIMING
            glQueryCounter(state.queries[state.qCount++], GL_TIMESTAMP);
            glDrawArrays(GL_TRIANGLES, i*30, 6);
            glQueryCounter(state.queries[state.qCount++], GL_TIMESTAMP);
            #else
            glDrawArrays(GL_TRIANGLES, i*30, 6);
            #endif
        }

        static float const baseColor[] = {0.f, 0.f, 0.f, 1.f};
        glUniform4fv(state.renderData.uButtonActiveColorLocation, 1, baseColor);

        for (size_t i = 0; i < UI.buttons.size(); i++) {

            #ifdef ENABLE_TIMING
            glQueryCounter(state.queries[state.qCount++], GL_TIMESTAMP);
            glDrawArrays(GL_TRIANGLES, (i*30)+6, 24);
            glQueryCounter(state.queries[state.qCount++], GL_TIMESTAMP);
            #else
            glDrawArrays(GL_TRIANGLES, (i*30)+6, 24);
            #endif
        }


        // Cleanup
        glBindVertexArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, 0 );

        glDisable(GL_BLEND);
        glDisable( GL_PROGRAM_POINT_SIZE );

        OGL_CHECKPOINT_DEBUG();

        
        
        #ifdef ENABLE_TIMING

        auto totalF2F = std::chrono::high_resolution_clock::now() - state.startF2F;
        
        GLuint64 start1_2, end1_2, start1_4_1, end1_4_1, start1_4_2, end1_4_2, start1_5, end1_5, startUI_1, endUI_1, startUI_2, endUI_2;
        glGetQueryObjectui64v(state.queries[0], GL_QUERY_RESULT, &start1_2);
        glGetQueryObjectui64v(state.queries[1], GL_QUERY_RESULT, &end1_2);
        glGetQueryObjectui64v(state.queries[2], GL_QUERY_RESULT, &start1_4_1);
        glGetQueryObjectui64v(state.queries[3], GL_QUERY_RESULT, &end1_4_1);
        glGetQueryObjectui64v(state.queries[4], GL_QUERY_RESULT, &start1_4_2);
        glGetQueryObjectui64v(state.queries[5], GL_QUERY_RESULT, &end1_4_2);
        glGetQueryObjectui64v(state.queries[6], GL_QUERY_RESULT, &start1_5);
        glGetQueryObjectui64v(state.queries[7], GL_QUERY_RESULT, &end1_5);

        glGetQueryObjectui64v(state.queries[8], GL_QUERY_RESULT, &startUI_1);
        glGetQueryObjectui64v(state.queries[9], GL_QUERY_RESULT, &endUI_1);
        glGetQueryObjectui64v(state.queries[10], GL_QUERY_RESULT, &startUI_2);
        glGetQueryObjectui64v(state.queries[11], GL_QUERY_RESULT, &endUI_2);

        auto totalGPUtime = (end1_2 - start1_2) +
                            (end1_4_1 - start1_4_1) +
                            (end1_4_2 - start1_4_2) +
                            (end1_5 - start1_5) +
                            (endUI_1 - startUI_1) +
                            (endUI_2 - startUI_2);

        auto totalCPUtime = totalF2F - std::chrono::nanoseconds(totalGPUtime);

        printf("Per Frame Total Render Time, GPU: %lu ns\n", totalGPUtime);
        printf("1.2 Render Time, GPU: %lu ns\n", end1_2 - start1_2);
        printf("1.4 Render Time, GPU: %lu ns\n", (end1_4_1 - start1_4_1) + (end1_4_2 - start1_4_2));
        printf("1.5 Render Time, GPU: %lu ns\n", end1_5 - start1_5);

        printf("Frame-to-Frame Time, CPU: %lu ns\n", totalF2F.count());
        printf("Time to submit Rendering, CPU: %lu ns\n", totalCPUtime.count());

        glDeleteQueries(12, state.queries);

        state.qCount = 0;

        #endif

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

    void initialisePointLights(State_& state) {
        Vec3f origin = state.vehicleControl.origin;

        // Evenly spaced positions around the ship
        float radius = 0.5f; // Distance from the ship's origin
        Vec3f lightPos1 = Vec3f{origin.x + radius, origin.y + 0.3f, origin.z};
        Vec3f lightPos2 = Vec3f{origin.x - radius * 0.5f, origin.y + 0.3f, origin.z + radius * std::sqrt(3.f) / 2.f};
        Vec3f lightPos3 = Vec3f{origin.x - radius * 0.5f, origin.y + 0.3f, origin.z - radius * std::sqrt(3.f) / 2.f};

        // RGB colors for the lights
        Vec3f red   = Vec3f{1.0f, 0.0f, 0.0f};
        Vec3f green = Vec3f{0.0f, 1.0f, 0.0f};
        Vec3f blue  = Vec3f{0.0f, 0.0f, 1.0f};

        state.renderData.lights = {
            Light{
                lightPos1,
                red,                    // Diffuse
                red * 0.5f,             // Specular
                red * 0.3f,             // Ambient
                lightPos1 - origin      // Offset from the ship
            },
            Light{
                lightPos2,
                green,              
                green * 0.5f,       
                green * 0.3f,       
                lightPos2 - origin  
            },
            Light{
                lightPos3,
                blue,               
                blue * 0.5f,        
                blue * 0.3f,        
                lightPos3 - origin  
            }
        };
    }

    void update_camera_pos( State_& state ) {
        if ( state.camControl.camView != FREE_ROAM && state.camControl2.camView != FREE_ROAM )
        	return;

        float speedModifier = state.freeRoamCtrls.moveFast ? 2.f : state.freeRoamCtrls.moveSlow ? 0.5f : 1.f;
        float velocity = kMovementPerSecond_ * state.deltaTime * speedModifier;

        // Forward / Backward
        if (state.freeRoamCtrls.movingForward)
        	state.freeRoamCtrls.cameraPos += velocity * state.freeRoamCtrls.cameraFront;
        if (state.freeRoamCtrls.movingBackward)
        	state.freeRoamCtrls.cameraPos -= velocity * state.freeRoamCtrls.cameraFront;

        // Left / Right
        if (state.freeRoamCtrls.strafingLeft)
			// Use cross product to create the 'right vector' then move along that
			state.freeRoamCtrls.cameraPos -= normalize(
				cross(state.freeRoamCtrls.cameraFront, state.freeRoamCtrls.cameraUp)
			) * velocity;
        if (state.freeRoamCtrls.strafingRight)
			state.freeRoamCtrls.cameraPos += normalize(
				cross(state.freeRoamCtrls.cameraFront, state.freeRoamCtrls.cameraUp)
			) * velocity;

        // Up / Down
        // Compute the camera's local "upward" direction
        Vec3f cameraRelativeUp = normalize(cross(
            cross(state.freeRoamCtrls.cameraFront, state.freeRoamCtrls.cameraUp), 
            state.freeRoamCtrls.cameraFront
        ));

        if (state.freeRoamCtrls.movingUp)
            state.freeRoamCtrls.cameraPos += velocity * cameraRelativeUp;
        if (state.freeRoamCtrls.movingDown)
            state.freeRoamCtrls.cameraPos -= velocity * cameraRelativeUp;
    

        // If update the camera to the free roam view
        if (state.camControl.camView == FREE_ROAM) {
            state.camControl.cameraPos = state.freeRoamCtrls.cameraPos;
            state.camControl.cameraUp = state.freeRoamCtrls.cameraUp;
            state.camControl.cameraFront = state.freeRoamCtrls.cameraFront;
        }

        if (state.camControl2.camView == FREE_ROAM) {
            state.camControl2.cameraPos = state.freeRoamCtrls.cameraPos;
            state.camControl2.cameraUp = state.freeRoamCtrls.cameraUp;
            state.camControl2.cameraFront = state.freeRoamCtrls.cameraFront;
        }
        // std::printf("%f, %f, %f\n", state.freeRoamCtrls.cameraPos.x, state.freeRoamCtrls.cameraPos.y, state.freeRoamCtrls.cameraPos.z);
    }

    void configureCamera( State_& state ) {

		    auto cameras = { &state.camControl, &state.camControl2 }; // Store pointers to the cameras
		
        // Loop through each camera
        for (auto* cam : cameras) {
          if (cam->camView == FIXED_DISTANCE) {
            cam->cameraPos = state.vehicleControl.position + Vec3f{ 1.f, 3.f, -3.f };
            cam->cameraFront = normalize(state.vehicleControl.position - cam->cameraPos);
            cam->cameraUp = { 0.f, 1.f, 0.f };
          } 
          else if (cam->camView == GROUND_POSITION) {
            cam->cameraPos = Vec3f{ 0.f, 0.5f, 0.f };
            cam->cameraFront = normalize(state.vehicleControl.position - cam->cameraPos);
            cam->cameraUp = { 0.f, 1.f, 0.f };
          }
        }

            update_camera_pos( state );
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

        #ifdef ENABLE_TIMING
		glQueryCounter(state.queries[state.qCount++], GL_TIMESTAMP);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
		glQueryCounter(state.queries[state.qCount++], GL_TIMESTAMP);
        #else
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        #endif
    }


    // Contains main rendering logic
    void renderScene( State_ &state ) {

        // === Setting up models ===
        // Langerso translations
        Mat44f model2world = kIdentity44f;
        Mat44f projCameraWorld = state.renderData.projection * state.renderData.world2camera * model2world;
        Mat33f normalMatrix = mat44_to_mat33(transpose(invert(model2world)));

        // Translations and projection for first launchpad
        Mat44f model2worldLaunchpad =  make_translation( Vec3f { 3.f, 0.f, -5.f } ) * model2world;
        Mat44f projCameraWorld_LP1 = state.renderData.projection * state.renderData.world2camera * model2worldLaunchpad;
        Mat33f normalMatrix_LP1 = mat44_to_mat33(transpose(invert(model2worldLaunchpad)));
        
        // Translations and projection for second launchpad
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

        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, state.renderData.textureObjectId );

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

        // Draw second launch pad
        glUniformMatrix4fv(
            state.renderData.uModel2WorldLocation, 1,
            GL_TRUE, model2worldLaunchpad2.v
        );

        drawMesh(state.renderData.landingPadVao, state.renderData.landingPadVertexCount, projCameraWorld_LP2, normalMatrix_LP2, state);

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
            if (aAction == GLFW_PRESS && aKey == GLFW_KEY_C)
            {
                // Cycle through camera views
                if (aMod == GLFW_MOD_SHIFT) {
                    ++state->camControl2.camView;
                    state->camControl2.camView %= 3;
                } else {
                    ++state->camControl.camView;
                    state->camControl.camView %= 3;
                }
            }

            if (aAction == GLFW_PRESS && aKey == GLFW_KEY_F) {
                state->vehicleControl.launch ^= true;
                if (!state->vehicleControl.hasLaunched)
                    state->vehicleControl.hasLaunched = true;

            }

            if (aAction == GLFW_PRESS && aKey == GLFW_KEY_R) {
                state->vehicleControl.resetVehicle();

                // Reset lights
                initialisePointLights( *state );

                state->particleSystem->reset( state->vehicleControl.origin );
            }

            if (aAction == GLFW_PRESS && aKey == GLFW_KEY_V) { state->isSplitScreen = !state->isSplitScreen; }

            if (aAction == GLFW_PRESS || aAction == GLFW_RELEASE)
            {
                bool isPressed = (aAction == GLFW_PRESS);

                // Move when pressed
                switch (aKey) {
                    case GLFW_KEY_W:
                    state->freeRoamCtrls.movingForward = isPressed;
                    break;
                    case GLFW_KEY_S:
                    state->freeRoamCtrls.movingBackward = isPressed;
                    break;
                    case GLFW_KEY_A:
                    state->freeRoamCtrls.strafingLeft = isPressed;
                    break;
                    case GLFW_KEY_D:
                    state->freeRoamCtrls.strafingRight = isPressed;
                    break;
                    case GLFW_KEY_E:
                    state->freeRoamCtrls.movingUp = isPressed;
                    break;
                    case GLFW_KEY_Q:
                    state->freeRoamCtrls.movingDown = isPressed;
                    break;
                    // Not sure if it's better to use aMod for these?
                    case GLFW_KEY_LEFT_SHIFT:
                    case GLFW_KEY_RIGHT_SHIFT:
                    state->freeRoamCtrls.moveFast = isPressed;
                    break;
                    case GLFW_KEY_LEFT_CONTROL:
                    case GLFW_KEY_RIGHT_CONTROL:
                    state->freeRoamCtrls.moveSlow = isPressed;
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

            if (state->freeRoamCtrls.cameraActive && (state->camControl.camView == FREE_ROAM || state->camControl2.camView == FREE_ROAM)) {
                float dx = (float)aMouseXPos - fbwidth/2.f;
                float dy = (float)aMouseYPos - fbheight/2.f;

                // Update pitch and yaw
                // Multiplying by deltaTime ensures smooth camera movement independent of framerate
                state->freeRoamCtrls.yaw += dx * kMouseSensitivity_ * (float)state->deltaTime;
                state->freeRoamCtrls.pitch += dy * kMouseSensitivity_ * (float)state->deltaTime;

                // Clamp pitch
                const float maxPitch =  89.f * std::numbers::pi_v<float> / 180.f;
                if (state->freeRoamCtrls.pitch > maxPitch)
                state->freeRoamCtrls.pitch = maxPitch;
                else if (state->freeRoamCtrls.pitch < -maxPitch)
                state->freeRoamCtrls.pitch = -maxPitch;


                // This prevents the mouse moving off-screen
                glfwSetCursorPos( aWindow, fbwidth/2.f, fbheight/2.f );

                float cosYaw = std::cos(state->freeRoamCtrls.yaw);
                float sinYaw = std::sin(state->freeRoamCtrls.yaw);
                float cosPitch = std::cos(state->freeRoamCtrls.pitch);
                float sinPitch = std::sin(state->freeRoamCtrls.pitch);

                Vec3f direction = {};
                direction.x = cosYaw * cosPitch;
                direction.y = -sinPitch;
                direction.z = sinYaw * cosPitch;

                state->freeRoamCtrls.cameraFront = normalize(direction);

                // Update each camera accordingly
                if (state->camControl.camView == FREE_ROAM)
                    state->camControl.cameraFront = state->freeRoamCtrls.cameraFront;
                if (state->camControl2.camView == FREE_ROAM)
                    state->camControl2.cameraFront = state->freeRoamCtrls.cameraFront;

            }

			else {

				for (auto& b : UI.buttons) {
					// Convert corner1 and corner2 from NDC to screen space
					float corner1X, corner2X, corner1Y, corner2Y;

					corner1X = (b.corner1.x + 1.0f) * 0.5f * fbwidth; // NDC to screen X
					corner2X = (b.corner2.x + 1.0f) * 0.5f * fbwidth; // NDC to screen X

					corner1Y = fbheight - (b.corner1.y + 1.0f) * 0.5f * fbheight; // NDC to screen Y (top-left origin)
					corner2Y = fbheight - (b.corner2.y + 1.0f) * 0.5f * fbheight; // NDC to screen Y (top-left origin)

                    // Check if the mouse position (screen space) is within the button boundaries
                    if (corner1X <= aMouseXPos && aMouseXPos <= corner2X &&
                        corner1Y <= aMouseYPos && aMouseYPos <= corner2Y) {
                        b.state = MOUSE_OVER;
                    } else {
                        b.state = NORMAL;
                    }
                }

            }
        }
    }

    void glfw_callback_mouse_( GLFWwindow* aWindow, int aButton, int aAction, int aMods )
    {
        if ( auto* state = static_cast<State_*>(glfwGetWindowUserPointer( aWindow )) )
        {
            if( GLFW_MOUSE_BUTTON_RIGHT == aButton && GLFW_PRESS == aAction ) {
                // Toggle camera control
                state->freeRoamCtrls.cameraActive = !state->freeRoamCtrls.cameraActive;

                // Hide / Show cursor
                if (state->freeRoamCtrls.cameraActive)
                	glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                else
                	glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            if( GLFW_MOUSE_BUTTON_LEFT == aButton && GLFW_PRESS == aAction ) {

                for (auto& b : UI.buttons) {
                    if (b.state == MOUSE_OVER) {

                        b.state = PRESSED;
                        if (b.text == "Launch") {
                            // toggle the launch
                            state->vehicleControl.launch ^= true;

                            if (!state->vehicleControl.hasLaunched)
                                state->vehicleControl.hasLaunched = true;
                        }
                        else if (b.text == "Reset") {
                            state->vehicleControl.resetVehicle();
                            initialisePointLights( *state );

                            state->particleSystem->reset( state->vehicleControl.origin );
                        }

                    }
                }
            }
            if( GLFW_MOUSE_BUTTON_LEFT == aButton && GLFW_RELEASE == aAction ) {

                for (auto& b : UI.buttons) {
                    if (b.state == PRESSED) {
                        b.state = MOUSE_OVER;
                    }
                }
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
