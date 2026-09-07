#pragma once

#include "GLOBALS.h"

#if (ENABLE_GUI)
#include <functional>

#include <GL/glew.h>

#include "imgui.h"
#include "imgui_impl_opengl3.h"

#if (USE_GLFW)
#include <GLFW/glfw3.h>

#include "imgui_impl_glfw.h"
#else
#include <SDL.h>
#undef main
#include "imgui_impl_sdl2.h"
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif
#endif

#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "VertexBufferLayout.h"
#include "Texture.h"

#if (USE_GLFW)
typedef GLuint texture_id_t; 
#else
typedef uint32_t texture_id_t; 
#endif

#define INVALID_TEXTURE_ID 0

#define TITLE_TEXT_SCALING 1.5f

// The model is that only gui_core will write to this state
// except for the fields leftClicked, rightClicked, dragEnded;
// these will only ever be set 'true' by gui_core.
// Client code should check leftClicked, rightClicked, dragEnded 
// after it calls renderImageInImGui, and handle whichever events
// it needs to respond to. If it responds to an event, the 
// client code has the task of setting back to 'false'
// when it has handled the event so that it does not process the same event twice.
//
// The above comment is was written before I put the ImageMouseHandling global 
// state into well protected classes, so now the stuff it explains is inviolable. 
// The original design required client code cooperation to enforce, and that is no longer true,
// but I am leaving the original comment because I think it explains the model well. 
//
// FYI:
// None of this state is threadsafe, this is only to be used in the gui thread
class InternalImageMouseHandlingState;

class ImageMouseHandlingInterface
{
protected:
	friend class InternalImageMouseHandlingState;
    bool    imagePosInit                 = false;
    bool    isDragging                   = false;
    bool    hovered                      = false;
    bool    leftClicked                  = false;
    bool    rightClicked                 = false;
    bool    dragEnded                    = false;
    ImVec2  mousePos                     = {0.0f, 0.0f};
    ImVec2  itemPos                      = {0.0f, 0.0f};
    ImVec2  relativePos                  = {0.0f, 0.0f};
    ImVec2  relativeDragStart            = {0.0f, 0.0f};
    ImVec2  relativeDragOffset           = {0.0f, 0.0f};
    ImVec2  relativeDragStartNormalized  = {0.0f, 0.0f};
    ImVec2  relativeDragOffsetNormalized = {0.0f, 0.0f};
    ImVec2  normalized                   = {0.0f, 0.0f};
    float   scrollOffset                 = 0.0f;
public:
    inline bool    get_imagePosInit()                 const { return this->imagePosInit; };
    inline bool    get_isDragging()                   const { return this->isDragging; };
    inline bool    get_hovered()                      const { return this->hovered; };
    inline bool    get_leftClicked()                  const { return this->leftClicked; };
    inline bool    get_rightClicked()                 const { return this->rightClicked; };
    inline bool    get_dragEnded()                    const { return this->dragEnded; };
    inline bool    consume_leftClicked()              { bool ret = this->leftClicked; this->leftClicked = false; return ret; };
    inline bool    consume_rightClicked()             { bool ret = this->rightClicked; this->rightClicked = false; return ret; };
    inline bool    consume_dragEnded()                { bool ret = this->dragEnded; this->dragEnded = false; return ret; };
    inline ImVec2  get_mousePos()                     const { return this->mousePos; };
    inline ImVec2  get_itemPos()                      const { return this->itemPos; };
    inline ImVec2  get_relativePos()                  const { return this->relativePos; };
    inline ImVec2  get_relativeDragStart()            const { return this->relativeDragStart; };
    inline ImVec2  get_relativeDragOffset()           const { return this->relativeDragOffset; };
    inline ImVec2  get_relativeDragStartNormalized()  const { return this->relativeDragStartNormalized; };
    inline ImVec2  get_relativeDragOffsetNormalized() const { return this->relativeDragOffsetNormalized; };
    inline ImVec2  get_normalized()                   const { return this->normalized; };
    inline float   get_scrollOffset()                 const { return this->scrollOffset; };
};

extern ImageMouseHandlingInterface g_imageMouseState;

void displayImageMouseState();

bool ScrollableButton(const std::string&, float, float, float);

bool FocusableButton(const char* label, const ImVec2& size = ImVec2(0, 0), bool* isFocused = nullptr);

void scaledImGuiText(const string& , float );

#define GenericTitleText(txt) (scaledImGuiText(string(txt), TITLE_TEXT_SCALING))

// Global texture storage
extern std::vector<std::pair<string, texture_id_t>> textureStore;

void renderImageInImGui(bool updateNeeded, const float* data, const string& name, int width, int height);

void updateUserInputScalar(const string&, size_t&, bool (*)(const size_t&));

void updateUserInputInt(const string&, int& , bool (*)(const int&));

void updateUserInputDouble(const string&, double&, bool (*)(const double&));

void SetNextUserInputStringWidth(float );

using UserInputToken = uint64_t;

UserInputToken ObtainUserInputToken(const string&);

bool updateUserInputString(const UserInputToken&, string&, bool (*)(const string&));

void removeDirectoryContents(const std::filesystem::path&);

void removeFile(const std::filesystem::path&); 

// Function to find a texture by name in the global store
texture_id_t findTextureByName(const string&);

void setTerminateGuiMain();

#if !MOVE_GUI_TO_COROUTINE_MODEL
void gui_main(string windowTitle, std::function<void(void)> guiLoopCallback);
#else
bool gui_main(string windowTitle);
#endif

#endif //(ENABLE_GUI)
