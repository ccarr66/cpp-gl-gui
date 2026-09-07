#include <gui_core.h>

#if (ENABLE_GUI)
#include "FileObj.h"

#include <iostream>
#include <sstream> 

ImageMouseHandlingInterface g_imageMouseState = { } ;

class InternalImageMouseHandlingState
{
public:
    static inline bool&    m_imagePosInit()                       { return g_imageMouseState.imagePosInit; };
	static inline bool&    m_isDragging()                         { return g_imageMouseState.isDragging; };
	static inline bool&    m_hovered()                            { return g_imageMouseState.hovered; };
	static inline bool&    m_leftClicked()                        { return g_imageMouseState.leftClicked; };
	static inline bool&    m_rightClicked()                       { return g_imageMouseState.rightClicked; };
	static inline bool&    m_dragEnded()                          { return g_imageMouseState.dragEnded; };
	static inline ImVec2&  m_mousePos()                           { return g_imageMouseState.mousePos; };
	static inline ImVec2&  m_itemPos()                            { return g_imageMouseState.itemPos; };
	static inline ImVec2&  m_relativePos()                        { return g_imageMouseState.relativePos; };
	static inline ImVec2&  m_relativeDragStart()                  { return g_imageMouseState.relativeDragStart; };
	static inline ImVec2&  m_relativeDragOffset()                 { return g_imageMouseState.relativeDragOffset; };
	static inline ImVec2&  m_relativeDragStartNormalized()        { return g_imageMouseState.relativeDragStartNormalized; };
	static inline ImVec2&  m_relativeDragOffsetNormalized()       { return g_imageMouseState.relativeDragOffsetNormalized; };
	static inline ImVec2&  m_normalized()                         { return g_imageMouseState.normalized; };
	static inline float&   m_scrollOffset()                       { return g_imageMouseState.scrollOffset; };
};

void displayImageMouseState()
{
    if (InternalImageMouseHandlingState::m_imagePosInit())
    {
        // Output coordinates
        ImGui::Text("Clicked at: %.1f, %.1f \n(normalized: %.3f, %.3f)", 
                InternalImageMouseHandlingState::m_relativePos().x, 
                InternalImageMouseHandlingState::m_relativePos().y, 
                InternalImageMouseHandlingState::m_normalized().x, 
                InternalImageMouseHandlingState::m_normalized().y);
        ImGui::Text("Hover = %s", ((InternalImageMouseHandlingState::m_hovered())? "true" : "false"));
        ImGui::Text("Left Clicked = %s", ((InternalImageMouseHandlingState::m_leftClicked())? "true" : "false"));
        ImGui::Text("Right Clicked = %s", ((InternalImageMouseHandlingState::m_rightClicked())? "true" : "false"));
        ImGui::Text("Drag Ended = %s", ((InternalImageMouseHandlingState::m_dragEnded())? "true" : "false"));
        ImGui::Text("Drag start at: %.1f, %.1f \n(normalized: %.3f, %.3f)", 
                InternalImageMouseHandlingState::m_relativeDragStart().x, 
                InternalImageMouseHandlingState::m_relativeDragStart().y, 
                InternalImageMouseHandlingState::m_relativeDragStartNormalized().x, 
                InternalImageMouseHandlingState::m_relativeDragStartNormalized().y);
        ImGui::Text("Drag Offset: %.1f, %.1f \n(normalized: %.3f, %.3f)", 
                InternalImageMouseHandlingState::m_relativeDragOffset().x, 
                InternalImageMouseHandlingState::m_relativeDragOffset().y, 
                InternalImageMouseHandlingState::m_relativeDragOffsetNormalized().x, 
                InternalImageMouseHandlingState::m_relativeDragOffsetNormalized().y);
    }
    else
    {
        ImGui::Text("Hover over any image to start.");
    }
}



bool ScrollableButton(const std::string& inputtext, float scrollSpeed, float width, float height)
{
    static float offset = 0.0f; // Static offset for scrolling
    
    string text = inputtext + string("  "); //spacing between start/end of wrap
    float textWidth = ImGui::CalcTextSize(text.c_str()).x;
    bool buttonClicked = false;

    // Update the scrolling offset
    offset += scrollSpeed * ImGui::GetIO().DeltaTime;
    if (offset > textWidth) offset = 0.0f; // Wrap around when text fully scrolls

    // Create a button with the specified width and height
    if (ImGui::Button("##ScrollableButton", ImVec2(width, height))) 
    {
        buttonClicked = true; // Button clicked
    }

    // Render scrolling text inside the button
    ImVec2 textStartPos = ImGui::GetItemRectMin(); // Top-left of the button
    ImVec2 textEndPos = ImGui::GetItemRectMax();   // Bottom-right of the button

    // Apply clipping to ensure the text stays within the button
    ImGui::PushClipRect(textStartPos, textEndPos, true);

    // Draw the scrolling text with an offset
    ImGui::GetWindowDrawList()->AddText(
        ImVec2(textStartPos.x - offset, textStartPos.y + (height - ImGui::CalcTextSize(text.c_str()).y) / 2), // Center vertically
        ImGui::GetColorU32(ImGuiCol_Text),
        text.c_str()
    );

    // Wrap the text by drawing it again at the end
    if (textWidth - offset < width) 
    {
        ImGui::GetWindowDrawList()->AddText(
            ImVec2(textStartPos.x + textWidth - offset, textStartPos.y + (height - ImGui::CalcTextSize(text.c_str()).y) / 2),
            ImGui::GetColorU32(ImGuiCol_Text),
            text.c_str()
        );
    }

    ImGui::PopClipRect();

    return buttonClicked;
}

void updateUserInputScalar(const string& label, size_t& scalarVal, bool (*validate)(const size_t&))
{
	size_t localScalarVal = scalarVal;
    
    // Style backup for resetting later
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4 originalColor = style.Colors[ImGuiCol_FrameBg];

    // Change text field color based on validation
    if (!validate(localScalarVal)) 
    {
        style.Colors[ImGuiCol_FrameBg] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red for invalid input
    }

    if (ImGui::InputScalar(label.c_str(), ImGuiDataType_U64, &localScalarVal)) 
    {
		scalarVal = localScalarVal;

        // Validate again after input update
        if (!validate(scalarVal)) 
        {
            style.Colors[ImGuiCol_FrameBg] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red for invalid input
        }
    }

    // Reset style
    style.Colors[ImGuiCol_FrameBg] = originalColor;

    return; 
}


void updateUserInputInt(const string& label, int& intVal, bool (*validate)(const int&))
{
	int localIntVal = intVal;
    
    // Style backup for resetting later
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4 originalColor = style.Colors[ImGuiCol_FrameBg];

    // Change text field color based on validation
    if (!validate(localIntVal)) 
    {
        style.Colors[ImGuiCol_FrameBg] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red for invalid input
    }

    if (ImGui::InputInt(label.c_str(),  &localIntVal)) 
    {
		intVal = localIntVal;

        // Validate again after input update
        if (!validate(intVal)) 
        {
            style.Colors[ImGuiCol_FrameBg] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red for invalid input
        }
    }


    // Reset style
    style.Colors[ImGuiCol_FrameBg] = originalColor;

    return; 
}


void updateUserInputDouble(const string& label, double& doubleVal, bool (*validate)(const double&))
{
	double localDoubleVal = doubleVal;
    
    // Style backup for resetting later
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4 originalColor = style.Colors[ImGuiCol_FrameBg];

    // Change text field color based on validation
    if (!validate(localDoubleVal)) 
    {
        style.Colors[ImGuiCol_FrameBg] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red for invalid input
    }

    if (ImGui::InputDouble(label.c_str(),  &localDoubleVal, 0.0, 0.0, "%.20f")) 
    {
		doubleVal = localDoubleVal;

        // Validate again after input update
        if (!validate(doubleVal)) 
        {
            style.Colors[ImGuiCol_FrameBg] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red for invalid input
        }
    }

    // Reset style
    style.Colors[ImGuiCol_FrameBg] = originalColor;

    return; 
}

enum class NextUserInputStringWidthStateType {None, Set};
static NextUserInputStringWidthStateType stateNextUserInputStringWidth;
static float nextUserInputStringWidth = 0.0;

void SetNextUserInputStringWidth(float width)
{
    if (stateNextUserInputStringWidth == NextUserInputStringWidthStateType::None)
    {
        stateNextUserInputStringWidth = NextUserInputStringWidthStateType::Set;
		nextUserInputStringWidth = width;
    }
    else
    {
        ErrorHandler::FatalError("Either double call or threading issue, please figure it out. - SetNextUserInputStringWidth");
    }
}

static const string DEFAULT_UI_STATE_FILENAME = "gui_state.txt";
static constexpr size_t MAX_NUM_UI_ELEMENTS = 0xffff;

static bool userInputLabelsAreLoaded = false;
static std::vector<string> userInputLabels;

UserInputToken ObtainUserInputToken(const string& newLabel)
{
    userInputLabels.push_back(newLabel);
    return UserInputToken{userInputLabels.size()-1};
}

const string& getUILabel(const UserInputToken& token)
{
    if ((size_t)token < userInputLabels.size())
    {
        return userInputLabels[token];
    }
    else 
    {
        throw std::runtime_error("Invalid UI Label");
    }
}

static inline std::filesystem::path getUIStatePath()
{
    return app_info::getExecutablePath().parent_path() / DEFAULT_UI_STATE_FILENAME;
}

void updateUIStateOnDisk()
{
    static const auto path = getUIStatePath();
    auto ofs = FileObjOutputOverwriteHandle(HEADER_NOT_NEEDED, path, TEXT_FILE_MODE);

    ofs << (size_t)userInputLabels.size() << "\n";
    for (auto label : userInputLabels) {
        ofs << label << "\n";
    }
	ofs.close();

    if (!fileExists(path))
    {
        ErrorHandler::FatalError("Failed to create UI state file!");
    }
}

void loadUIStateFromDisk()
{
    static const auto path = getUIStatePath();
    if ((!userInputLabelsAreLoaded)&&(fileExists(path)))
    {
        auto ifs = FileObjInputHandle(HEADER_NOT_NEEDED, path, TEXT_FILE_MODE);

        userInputLabels.clear();

        string numLabels_str;
        std::getline(ifs, numLabels_str);
        size_t numLabels = std::stoi(numLabels_str);

        if (numLabels <= MAX_NUM_UI_ELEMENTS) {
            for (size_t idx = 0; idx < numLabels; idx++) {
                string label;
                std::getline(ifs, label);
                userInputLabels.push_back(label);
            }
            userInputLabelsAreLoaded=true;
        }
        else
        {
            ErrorHandler::FatalError("Invalid UI Element count! Likely UI state is corrupt");
        }
        ifs.close();
    }
}

bool updateUserInputString(const UserInputToken& token, string& updatedString, bool (*validate)(const string&))
{
    const size_t BUF_SIZE = 256;  // Define a reasonable buffer size
    char buf[BUF_SIZE] = {0};
    //bool openPopup = false;
    bool stringUpdated = false;
    bool empty = updatedString.length()==0;

    if (empty)
    {
        loadUIStateFromDisk();
        try
        {
            updatedString = getUILabel(token);
        }
        catch (const std::runtime_error & e) 
        {
            ErrorHandler::SoftError(string(string("Post-UI-State-Reload failed to find UI label: ") + string(e.what())));
        }
    }
    
    bool tooLong = ((updatedString.length()+1) > (BUF_SIZE - 1)); //shouldnt happen
    if (!tooLong) 
    {
        // Determine the size to copy, ensuring no buffer overflow
        size_t copySize = std::min(updatedString.length()+1 /*for null term*/, BUF_SIZE - 1);

        // Copy from updatedString to buffer
        snprintf(buf, copySize, "%s", updatedString.c_str());
    }
    // Style backup for resetting later
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4 originalColor = style.Colors[ImGuiCol_FrameBg];

    // Change text field color based on validation
    if (tooLong || !validate(updatedString)) 
    {
        style.Colors[ImGuiCol_FrameBg] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red for invalid input
    }
    if (stateNextUserInputStringWidth == NextUserInputStringWidthStateType::Set && nextUserInputStringWidth > 0.0f)
    {
        stateNextUserInputStringWidth = NextUserInputStringWidthStateType::None;

        ImGui::SetNextItemWidth(nextUserInputStringWidth); 
        
        nextUserInputStringWidth = 0.0f;
    }

    const auto& start_text = getUILabel(token);

    if (ImGui::InputText(start_text.c_str(), buf, IM_ARRAYSIZE(buf))) 
    {
        // Check for potential truncation
        //if (buf[BUF_SIZE - 2] != '\0' || copySize >= BUF_SIZE-1) {
        //    openPopup=true;
        //}
        updatedString=string{buf};
        stringUpdated = true;
        // Validate again after input update
        if (!validate(updatedString)) 
        {
            style.Colors[ImGuiCol_FrameBg] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red for invalid input
        }
        
        updateUIStateOnDisk();
    }

    // Reset style
    style.Colors[ImGuiCol_FrameBg] = originalColor;

    return stringUpdated; //(openPopup);
} 

void removeDirectoryContents(const std::filesystem::path& directory) {
	if (!safeToRemove(directory)) {
		return;
	}
	for (const auto& entry : std::filesystem::directory_iterator(directory)) {
		std::filesystem::remove_all(entry);
	}
}


void removeFile(const std::filesystem::path& filePath) {
    if (!parentDirectorySafeToDelete(filePath)) {
        return;
    }
	std::filesystem::remove(filePath);
}


std::vector<std::pair<string, texture_id_t>> textureStore;

string getGLErrorString(GLenum error) 
{
    switch (error) 
    {
        case GL_NO_ERROR:                      return "No error";
        case GL_INVALID_ENUM:                  return "Invalid enum";
        case GL_INVALID_VALUE:                 return "Invalid value";
        case GL_INVALID_OPERATION:             return "Invalid operation";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "Invalid framebuffer operation";
        case GL_OUT_OF_MEMORY:                 return "Out of memory";
        case GL_STACK_UNDERFLOW:               return "Stack underflow";
        case GL_STACK_OVERFLOW:                return "Stack overflow";
        default:                               return "Unknown error";
    }
}

// Function to find a texture by name in the global store
texture_id_t findTextureIDByName(const string& name) 
{
    for (const auto& entry : textureStore) 
    {
        if (entry.first == name) 
        {
            return entry.second;
        }
    }
    return INVALID_TEXTURE_ID; // Return 0 if not found
}

texture_id_t findTextureIDByName(const string& name, size_t& idx) 
{
    idx=0;
    for (const auto& entry : textureStore) 
    {
        if (entry.first == name) 
        {
            return entry.second;
        }
        idx++;
    }
    return INVALID_TEXTURE_ID; // Return 0 if not found
}

texture_id_t createTexture(int width, int height, const float* data) 
{
    texture_id_t textureID = INVALID_TEXTURE_ID; 
            
    GLCall(glGenTextures(1, &textureID));
    if (textureID == INVALID_TEXTURE_ID)
    {
        GLenum error = glGetError();
		ErrorHandler::FatalError("TextureID should not be zero!\nGLError: " + getGLErrorString(error) + " (" + std::to_string(error) + ")");
    }
    GLCall(glBindTexture(GL_TEXTURE_2D, textureID));

    // Initialize an empty texture
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, data));

    // Set texture parameters
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ONE));

    GLCall(glBindTexture(GL_TEXTURE_2D, 0));
    
    return textureID;
}

void updateTexture(texture_id_t textureID, const float* data, int width, int height) 
{
    
    GLCall(glBindTexture(GL_TEXTURE_2D, textureID));
    GLCall(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED, GL_FLOAT, data));
    GLCall(glBindTexture(GL_TEXTURE_2D, 0));
    
}

void getTextureDimensions(texture_id_t textureID, int& width, int& height) 
{
     
    // Bind the texture to query its properties
    GLCall(glBindTexture(GL_TEXTURE_2D, textureID));
    
    // Retrieve the width of the texture
    GLCall(glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width));
    
    // Retrieve the height of the texture
    GLCall(glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height));
    
    // Unbind the texture to avoid side effects
    GLCall(glBindTexture(GL_TEXTURE_2D, 0));
    
}

void deleteTexture(texture_id_t& textureID) 
{
    if (textureID != INVALID_TEXTURE_ID) 
    {
        GLCall(glDeleteTextures(1, &textureID)); // Delete the texture from OpenGL
        textureID = INVALID_TEXTURE_ID; // Reset the texture ID to avoid dangling references
    }
}

void updateTextureInStore(const string& name, const texture_id_t textureID)
{
    size_t textureIdx=0;
    texture_id_t lookupTextureID = findTextureIDByName(name, textureIdx);

    
    if (lookupTextureID == INVALID_TEXTURE_ID)
    {
        textureStore.push_back({name, textureID});
    }
    else
    {
		try
		{
            textureStore[textureIdx].second = textureID;
		}
		catch (...) { 
            ErrorHandler::FatalError(string("Couldn't assign new textureID!: \ntextureIdx: ") + std::to_string(textureIdx) + string("\ntextureID: ") + std::to_string(textureID)); 
        }
    }

}

void renderImageInImGui(bool updateNeeded, const float* data, const string& name, int width, int height) {
    texture_id_t textureID = findTextureIDByName(name);
    if (textureID == INVALID_TEXTURE_ID)
    {
        textureID=createTexture(width, height, data);
		(void)updateTextureInStore(name, textureID);
    }
    else if (data)
    {
        int existingWidth, existingHeight;

        (void)getTextureDimensions(textureID, existingWidth, existingHeight);

        // Recreate texture if dimensions don't match
        if (existingWidth != width || existingHeight != height) 
		{
            (void)deleteTexture(textureID); // Delete the existing texture

            textureID = createTexture(width, height, data); // Create a new texture

            (void)updateTextureInStore(name, textureID); // Update the store with the new texture ID
        } 
        else if (updateNeeded)
		{
            // Update texture if dimensions match and data is provided
		    (void)updateTexture(textureID, data, width, height);
        }
    }

	// Get available space
	ImVec2 availableSpace = ImGui::GetContentRegionAvail();

	// Calculate aspect ratio
	float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

	// Scale dimensions to fit available space
	float scaledWidth = availableSpace.x;
	float scaledHeight = availableSpace.y;

	// Adjust scaling to maintain aspect ratio
	if (scaledWidth / aspectRatio <= scaledHeight) 
    {
		scaledHeight = scaledWidth / aspectRatio;
	}
	else 
    {
		scaledWidth = scaledHeight * aspectRatio;
	}

	//// Calculate padding for centering the image
	//float paddingX = (availableSpace.x - scaledWidth) / 2.0f;
	//float paddingY = (availableSpace.y - scaledHeight) / 2.0f;
//
	//// Add dummy spacing to center the image
	//ImGui::Dummy(ImVec2(0, paddingY));
	//if (paddingX > 0.0)
	//{
	//	ImGui::Dummy(ImVec2(paddingX, 0));
	//	ImGui::SameLine();
	//}

	// Render the scaled image
	ImGui::Image((ImTextureID)textureID, ImVec2(scaledWidth, scaledHeight));   

    // Check if the image is hovered
    if (InternalImageMouseHandlingState::m_hovered() || ImGui::IsItemHovered() || ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) 
    {
        // update scroll state
        ImGuiIO& io = ImGui::GetIO();
        InternalImageMouseHandlingState::m_scrollOffset() += io.MouseWheel * 0.02f; // Scale the scroll speed
        InternalImageMouseHandlingState::m_scrollOffset() = std::clamp(InternalImageMouseHandlingState::m_scrollOffset(), 0.0f, 1.0f); // Clamp scroll range

        // Get mouse position relative to the image
        InternalImageMouseHandlingState::m_mousePos() = ImGui::GetMousePos(); // Screen-space mouse position
        InternalImageMouseHandlingState::m_itemPos() = ImGui::GetItemRectMin(); // Top-left of the image in screen space
        InternalImageMouseHandlingState::m_relativePos() = { InternalImageMouseHandlingState::m_mousePos().x - InternalImageMouseHandlingState::m_itemPos().x, InternalImageMouseHandlingState::m_mousePos().y - InternalImageMouseHandlingState::m_itemPos().y };

        // Normalize coordinates
        InternalImageMouseHandlingState::m_normalized().x = InternalImageMouseHandlingState::m_relativePos().x / scaledWidth;
        InternalImageMouseHandlingState::m_normalized().y = InternalImageMouseHandlingState::m_relativePos().y / scaledHeight;

        InternalImageMouseHandlingState::m_imagePosInit() = true;

        InternalImageMouseHandlingState::m_hovered() = ImGui::IsItemHovered();
        if (InternalImageMouseHandlingState::m_hovered())
        {
            // Handle mouse down (start dragging)
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                InternalImageMouseHandlingState::m_relativeDragStart() = InternalImageMouseHandlingState::m_relativePos();
                InternalImageMouseHandlingState::m_relativeDragStartNormalized().x = InternalImageMouseHandlingState::m_normalized().x; // Record starting position
                InternalImageMouseHandlingState::m_relativeDragStartNormalized().y = InternalImageMouseHandlingState::m_normalized().y;
                InternalImageMouseHandlingState::m_isDragging() = true;
                InternalImageMouseHandlingState::m_leftClicked() = true; //state can be set false by handling code
            }

            // Handle mouse release (end dragging)
            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            {
                if (!InternalImageMouseHandlingState::m_dragEnded() && InternalImageMouseHandlingState::m_isDragging())
                {
                    InternalImageMouseHandlingState::m_relativeDragOffset() = { InternalImageMouseHandlingState::m_relativePos().x - InternalImageMouseHandlingState::m_relativeDragStart().x, InternalImageMouseHandlingState::m_relativePos().y - InternalImageMouseHandlingState::m_relativeDragStart().y };
                    InternalImageMouseHandlingState::m_relativeDragOffsetNormalized().x = InternalImageMouseHandlingState::m_relativeDragOffset().x / scaledWidth;
                    InternalImageMouseHandlingState::m_relativeDragOffsetNormalized().y = InternalImageMouseHandlingState::m_relativeDragOffset().y / scaledHeight;
                    InternalImageMouseHandlingState::m_dragEnded() = true; //state can be set false by handling code
                }
                InternalImageMouseHandlingState::m_isDragging() = false;
            }

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            {
                InternalImageMouseHandlingState::m_rightClicked() = true; //state can be set false by handling code
            }
        }

    } 
}


#if (USE_GLFW)    
static GLFWwindow* window;
#else
static SDL_Window* window;
static SDL_GLContext glContext;
static bool useFramerateLimiter;
#include <chrono>
#endif 


static auto lastTime = std::chrono::high_resolution_clock::now();
static constexpr int targetFPS = 60;
static constexpr auto frameDuration = std::chrono::milliseconds(1000 / targetFPS);

void limitFrameRate() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime);
    if (elapsedTime < frameDuration) {
        SDL_Delay(static_cast<uint32_t>((frameDuration - elapsedTime).count()));
    }
    lastTime = std::chrono::high_resolution_clock::now();
}

string getGLEWErrorStr(GLenum err)
{
	std::stringstream ss; 
	string str;
	ss << glewGetErrorString(err);
	ss >> str;
	return str;
}

bool terminateGuiMain=false;

bool commonGuiInit(string windowTitle)
{
#if (USE_GLFW)            
	/* Initialize the library */
	if (!glfwInit()) return false;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(960, 540, windowTitle.c_str(), NULL, NULL);
	if (!window)
	{
		GLCall(glfwTerminate());
		return false;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	//sync w/ vsync
	glfwSwapInterval(1);

#else

    if (SDL_Init(SDL_INIT_VIDEO) != 0) 
	{
        ErrorHandler::SoftError(string("SDL_Init Error: ") + string(SDL_GetError()));
        return false;

    }

    // Set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Create a window with an OpenGL context
    window = SDL_CreateWindow(windowTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,     1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) 
	{
        ErrorHandler::SoftError(string("SDL_CreateWindow Error: ") + string(SDL_GetError()));
        SDL_Quit();
        return false;
    }

    // Create the OpenGL context
    glContext = SDL_GL_CreateContext(window);
    if (!glContext) 
	{
        ErrorHandler::SoftError(string("SDL_GL_CreateContext Error: ") + string(SDL_GetError()));
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    // Enable vsync
    if (SDL_GL_SetSwapInterval(1) < 0) {
		ErrorHandler::SoftError(string("Error enabling VSync: ") + string(SDL_GetError()));
		ErrorHandler::SoftError(string("Trying adaptive VSync..."));

		if (SDL_GL_SetSwapInterval(-1) < 0) {
			ErrorHandler::SoftError(string("Adaptive VSync not supported: ") + string(SDL_GetError()));
			ErrorHandler::SoftError(string("Disabling VSync entirely. Using our frame rate limiter."));

			if (SDL_GL_SetSwapInterval(0) < 0) {
				ErrorHandler::SoftError(string("Error disabling VSync: ") + string(SDL_GetError()));
			}
			useFramerateLimiter = true;
		}
	}
#endif

	// Initialize GLEW
	glewExperimental = GL_TRUE; // Ensure modern OpenGL extensions are loaded
	GLenum err = glewInit();
	if (err != GLEW_OK) 
	{
		ErrorHandler::SoftError(string("Failed to initialize GLEW: ") + getGLEWErrorStr(err));
		
#if (USE_GLFW)  
		glfwDestroyWindow(window);
		glfwTerminate();
#else
		SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
#endif

		return false;
	}

    // Output OpenGL version
    GP::out << glGetString(GL_VERSION) << std::endl;
	GLCall(glEnable(GL_BLEND));
	GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    //Setup IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable keyboard navigation
    (void)io;
    ImGui::StyleColorsDark();
#if (USE_GLFW)  
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#else
    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
#endif
    ImGui_ImplOpenGL3_Init((char*)glGetString(GL_NUM_SHADING_LANGUAGE_VERSIONS));

    terminateGuiMain=false;

	return true;
}


void pollEvents()
{
#if (USE_GLFW)  
	glfwPollEvents();
	terminateGuiMain = glfwWindowShouldClose(window);
#else
    SDL_Event event;
	while (SDL_PollEvent(&event)) 
	{
		ImGui_ImplSDL2_ProcessEvent(&event); // Pass event to ImGui
		if (event.type == SDL_QUIT) 
		{
			setTerminateGuiMain();
		}
	}

	if (useFramerateLimiter)
	{
		limitFrameRate();
	}
#endif
}

void scaledImGuiText(const string& txt, float scale)
{
	ImGui::SetWindowFontScale(scale); // Temporarily scale text size
	ImGui::TextUnformatted(txt.c_str());
	ImGui::SetWindowFontScale(1.0f); // Reset scaling
}

void setTerminateGuiMain()
{
	terminateGuiMain = true;
}

int g_display_w, g_display_h;
void frameInit()
{
    pollEvents();

    // Start the ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
#if (USE_GLFW)  
    ImGui_ImplGlfw_NewFrame();
#else
    ImGui_ImplSDL2_NewFrame();
#endif
    ImGui::NewFrame();

    // Determine display size
    int g_display_w, g_display_h;
#if (USE_GLFW)  
    glfwGetFramebufferSize(window, &display_w, &display_h);
#else
    SDL_GetWindowSize(window, &g_display_w, &g_display_h);
#endif

    // Set the next window size and position to cover the entire screen
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(g_display_w), static_cast<float>(g_display_h)));
}

void render()
{
    // Rendering
    ImGui::Render();
#if (USE_GLFW)  
    glfwGetFramebufferSize(window, &g_display_w, &g_display_h);
#else
    SDL_GetWindowSize(window, &g_display_w, &g_display_h);
#endif
    glViewport(0, 0, g_display_w, g_display_h);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    #if (USE_GLFW)
    glfwSwapBuffers(window);
    #else
    SDL_GL_SwapWindow(window);
    #endif
}

void cleanup()
{
    try {
        ImGui_ImplOpenGL3_Shutdown();
#if (USE_GLFW)  
        ImGui_ImplGlfw_Shutdown();
#else
        ImGui_ImplSDL2_Shutdown();
#endif
        ImGui::DestroyContext();

#if (USE_GLFW)  
        glfwDestroyWindow(window);
        glfwTerminate();
#else
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
#endif
    }
    catch (const std::exception & e) {
        ErrorHandler::FatalError(string(string("Could not clean up successfully!: ") + string(e.what())));
    }
}

#if !MOVE_GUI_TO_COROUTINE_MODEL

void gui_main(string windowTitle, std::function<void(void)> guiLoopCallback)
{	
	if (commonGuiInit(windowTitle))
	{
		/* Loop until the user closes the window */
		while (!terminateGuiMain)
		{
			frameInit();

			guiLoopCallback();

            render();
		}
	}
    cleanup();
}

#else

#include <iostream>
#include <coroutine>
#include <optional>

enum class GuiState_t : uint32_t
{
    uninitialized = 0,
    invalid = 1,
    valid = 2,
    terminate = 3,
};

enum class GuiInternalValidState_t : uint32_t
{
    invalid = 0,
    frameInit = 1,
    rendering = 2,
};

struct GuiGenerator_t {
    struct promise_type {
        GuiState_t currentState;

        auto get_return_object() { return GuiGenerator_t{std::coroutine_handle<promise_type>::from_promise(*this)}; }
        auto initial_suspend() { return std::suspend_always{}; }
        auto final_suspend() noexcept { return std::suspend_always{}; }
        void return_void() {}
        auto yield_value(GuiState_t value) {
            currentState = value;
            return std::suspend_always{};
        }
        void unhandled_exception() {ErrorHandler::FatalError(string("Unhandled exception in coroutine!: "));}
    };

    std::coroutine_handle<promise_type> handle;

    GuiGenerator_t(std::coroutine_handle<promise_type> h) : handle(h) {}

    GuiGenerator_t(GuiGenerator_t&& other) noexcept : handle(std::exchange(other.handle, nullptr)) {}
    GuiGenerator_t& operator=(GuiGenerator_t&& other) noexcept 
    {
        if (this != &other) 
        {
            if (this->handle) this->handle.destroy();
            this->handle = std::exchange(other.handle, nullptr);
        }
        return *this;
    }

    ~GuiGenerator_t() 
    {
        if (handle)
        {
            handle.destroy();
        }
    }

    bool move_next() 
    { 
        if (handle.done()) 
        {
            return false;
        }
        handle.resume();
        // Now we must check if the coroutine is done after the resume:
        return !handle.done();
    }

    GuiState_t getCurrentState() { return handle.promise().currentState; }
};

static bool guiStateMachineInit = false;
static bool guiStateIsValid = false;
static std::optional<GuiGenerator_t> guiStateMachineGenerator;
static GuiState_t currentGuiState = GuiState_t::uninitialized;
static GuiInternalValidState_t currentValidState = GuiInternalValidState_t::frameInit;
static string g_windowTitle = "Title not set!";

GuiGenerator_t gui_state_machine() 
{
    while(true)
    {
        GuiState_t localState = currentGuiState;
        
        // This is defensive code, if currentGuiState is terminate, so should terminateGuiMain
        // I think I can remove terminateGuiMain in the future, keeping for easy
        //compatibilty with the rest of the pre-coroutine code
        terminateGuiMain = terminateGuiMain || localState == GuiState_t::terminate;

        if (localState == GuiState_t::uninitialized) 
        {
            localState = GuiState_t::invalid;

            if (commonGuiInit(g_windowTitle)) 
            {
                currentValidState = GuiInternalValidState_t::frameInit;
                localState = GuiState_t::valid;
            }
        }

        if (!terminateGuiMain && localState == GuiState_t::valid)
        {
            /* Loop until the user closes the window */
            GuiInternalValidState_t localValidState = GuiInternalValidState_t::invalid;
            GuiInternalValidState_t localErrorState = GuiInternalValidState_t::invalid;

            switch (currentValidState)
            {
                case GuiInternalValidState_t::rendering:
                    render();
                    localValidState = GuiInternalValidState_t::frameInit; //conceptual not functional, probably optimmized out idc
                    //deliberate fall through, need to prep next frame 
                    //for gui client code to do its stuff
                    [[fallthrough]];
                case GuiInternalValidState_t::frameInit:
                    frameInit();
                    localValidState = GuiInternalValidState_t::rendering;
                    break;
                default:
                    localValidState = GuiInternalValidState_t::invalid;
                    localErrorState = currentValidState;
                    break;
            }

            if (localValidState == GuiInternalValidState_t::invalid)
            {
                localState = GuiState_t::invalid;
                ErrorHandler::FatalError("We tried to set an invalid state!" + std::to_string(ECBR_ValTy(GuiInternalValidState_t, localErrorState)));
            }
            else if (localValidState == GuiInternalValidState_t::frameInit)
            {
                localState = GuiState_t::invalid;
                ErrorHandler::FatalError("We did frameInit but didnt render!");
            }
            else
            {
                currentValidState = localValidState;
            }
        }
        
        if (terminateGuiMain || localState != GuiState_t::valid)
        {
            localState = GuiState_t::terminate;
            cleanup();
        }

        co_yield localState;  // Suspend and yield a value
    }
}

bool gui_main(string windowTitle) 
{
    if(!guiStateMachineInit)
    {
        g_windowTitle = windowTitle;
        currentGuiState = GuiState_t::uninitialized;
        try 
        {
            guiStateMachineGenerator.emplace(gui_state_machine());
        }
        catch (const std::exception & e) 
        {
            ErrorHandler::FatalError(string(string("Could not create gui generator!: ") + string(e.what())));
        }

        guiStateMachineInit = (guiStateMachineGenerator.has_value());

        guiStateIsValid = guiStateMachineInit;
    }

    guiStateIsValid = guiStateIsValid && (guiStateMachineGenerator.has_value());
    
    if (guiStateIsValid && guiStateMachineGenerator->move_next()) 
    {
        currentGuiState = guiStateMachineGenerator->getCurrentState();
        guiStateIsValid = (currentGuiState == GuiState_t::terminate || currentGuiState == GuiState_t::valid);
    }

    if (!guiStateIsValid)
    {
        ErrorHandler::FatalError("Gui is in invalid state!");
    }
    
    return (currentGuiState == GuiState_t::valid);
}

#endif

#endif //(ENABLE_GUI)