// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <stdio.h>
#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
//#define GL_SILENCE_DEPRECATION
//#if defined(IMGUI_IMPL_OPENGL_ES2)
//#include <GLES2/gl2.h>
//#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

struct FormInput {
    char label[128];
    char name[128];
    char buff[256];
    double  value;
    bool hasError = false;
    char error[128];
    char type[128];
};

struct Appliance {
    char name[128];
    double power;
    double hours;
    double days;
    double totalPower;

    Appliance(const char* n, double p, double h, double d, double t) : power(p), hours(h), days(d), totalPower(t) {
        strcpy_s(name, sizeof(name), n);
    }
};

struct Submissions {
    char name[128];
    double totalCost;
    double totalEnergy;
    std::vector<Appliance> appliances;
};

static char name[128];
bool isEditing = false;
bool isNameSet = false;

std::vector<FormInput> formFields;
std::vector<Appliance> appliances;

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void initializeForm() {
    isNameSet = false;
    name[0] = '\0';

    formFields = {
        {"Appliance name", "name", "", 0, false, "", "text"},
        {"Power", "power", "", 0, false, "", "number"},
        {"Hours/day", "hours", "", 0, false, "", "number"},
        {"Days used","days", "", 0, false, "", "number"}
    };
    appliances.clear();
}

void handleError(const char* msg, FormInput &field) {
    field.hasError = true;
    char source[128];
    strcpy_s(source, msg);
    strcat_s(source, field.label);
    strcpy_s(field.error, source);
}

void addAppliance() {
    char name[128] = "";
    double power = NULL;
    double hours = NULL;
    double days = NULL;

    for (auto& field : formFields)
    {
        field.hasError = false;
        field.error[0] = '\0';

        if (strcmp("number", field.type) == 0) 
        {

            if (strcmp(field.name, "power") == 0)
                power = field.value;
            if (strcmp(field.name, "hours") == 0)
                hours = field.value;
            if (strcmp(field.name, "days") == 0)
                days = field.value;
            
        }
        else if (strcmp("text", field.type) == 0) 
        {
            if (field.buff[0] == '\0')
            {
                handleError("please enter a value for ", field);
            }
            if (strcmp("name", field.name) == 0) {
                strcpy_s(name, field.buff);
            }
        }
   
    }

    if (!power || !hours || !days || name[0] == '\0') return;

    double totalEnergy = power * hours * days;

    double totalCost = totalEnergy * 25;

    appliances.emplace_back(name, power, hours, days, totalEnergy);

    printf("total energy: %f \n total cost: %f", totalEnergy, totalCost);
}

void handleSubmit() {

}

// Main code
int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
    const char* glsl_version = "#version 300 es";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
    GLFWwindow* window = glfwCreateWindow((int)(1280 * main_scale), (int)(800 * main_scale), "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details. If you like the default font but want it to scale better, consider using the 'ProggyVector' from the same author!
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //style.FontSizeBase = 20.0f;
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    //IM_ASSERT(font != nullptr);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    float f = 0.0f;
    int counter = 0;

    initializeForm();

    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Dorm energy tracker");                          // Create a window called "Hello, world!" and append into it.
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

        if (!isNameSet || isEditing) 
        {
            ImGui::Spacing();
            ImGui::SeparatorText("Enter your Name");
            ImGui::Spacing();

            if (ImGui::BeginTable("NameTable", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerV))
            {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Name");

                ImGui::SetNextItemWidth(250);
                ImGui::TableSetColumnIndex(1);
                ImGui::InputText("##", name, IM_ARRAYSIZE(name));

                ImGui::TableSetColumnIndex(2);
                if (ImGui::Button("Submit", ImVec2(90, 25))) {
                    isNameSet = true;
                    isEditing = false;
                }

                ImGui::EndTable();
            }
           
        }
        else 
        {
            ImGui::Spacing();
            ImGui::SeparatorText("Appliance Details");
            ImGui::Spacing();

            ImGui::Text("%s's Power Usage", name);
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 120); 
            if (ImGui::Button(isEditing ? "Done" : "Edit", ImVec2(110, 25)))
            {
                isEditing = !isEditing;
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::BeginTable("FormTable", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerV))
            {
                for (auto& field : formFields)
                {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("%s", field.label);

                    ImGui::TableSetColumnIndex(1);
                    ImGui::PushID(field.label);
                    ImGui::SetNextItemWidth(250); 

                    if (strcmp(field.type, "number") == 0)
                        ImGui::InputDouble("##input", &field.value, 0, 0, "%.2f");
                    else
                        ImGui::InputText("##input", field.buff, IM_ARRAYSIZE(field.buff));

                    ImGui::PopID();

                    if (field.hasError)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(1);
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 80, 80, 255));
                        ImGui::TextWrapped("%s", field.error);
                        ImGui::PopStyleColor();
                    }
                }
                ImGui::EndTable();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::BeginGroup();
            if (ImGui::Button("Add Appliance", ImVec2(150, 30)))
            {
                addAppliance();
            }
            ImGui::SameLine();
            if (ImGui::Button("New Student", ImVec2(150, 30)))
            {
                initializeForm();
            }
            ImGui::SameLine();
            if (ImGui::Button("Submit", ImVec2(150, 30)))
            {
                handleSubmit();
            }
            ImGui::EndGroup();


            if (ImGui::BeginTable("Appliances", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("Power (w)");
                ImGui::TableSetupColumn("Hours/Day");
                ImGui::TableSetupColumn("Days Used");
                ImGui::TableSetupColumn("Total Power");
                ImGui::TableHeadersRow();

                for (const auto& a : appliances) 
                {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(a.name);

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%.2f", a.power);

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%.2f", a.hours);

                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%.2f", a.days);

                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text("%.2f", a.totalPower);
                }

                ImGui::EndTable();

            }

        }

        //ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

        //ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        //ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        //if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
        //    counter++;
        //ImGui::SameLine();
        //ImGui::Text("counter = %d", counter);

        ImGui::End();
        
        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}