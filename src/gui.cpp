#include "../headers/gui.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include "../headers/camera.hpp"


void Gui::imgui_Frame_Setup() 
{
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
}

void Gui::imgui_Init(GLFWwindow* window) 
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window,true);
    ImGui_ImplOpenGL3_Init("#version 460");
}
void Gui::imgui_Debug_Window(bool* is_Wireframe, glm::vec3 &planet_info, float &pickedAsteroidTheta, bool &ssao, glm::vec3 &radiusBiasPower, bool &shouldDrawOcean)
{
    float water_level = 0.5f;
    float mountain_start = 0.5f;
    float snow_peak_start = 0.5f;
    ImGui::Begin("Debug Window");
    if(ImGui::Checkbox("Wireframe Mode",is_Wireframe)) {}
    ImGui::SliderFloat("Water level",&planet_info[0],1.0f,2.0f);
    ImGui::SliderFloat("Mountain start",&planet_info[1],1.0f,2.0f);
    ImGui::SliderFloat("Snow peaks",&planet_info[2],1.0f,2.0f);
    ImGui::SliderFloat("Selected Asteroid Position", &pickedAsteroidTheta,-M_PI,M_PI);

    ImGui::Checkbox("SSAO", &ssao);
    ImGui::SliderFloat("SSAO radius",&radiusBiasPower[0],0.2f,0.6f);
    ImGui::SliderFloat("SSAO bias",&radiusBiasPower[1],0.01f,0.1f);
    ImGui::SliderFloat("SSAO power", &radiusBiasPower[2],1.0f,100.0f);

     ImGui::Checkbox("Render Ocean", &shouldDrawOcean);

    ImGui::End();
}
void Gui::imgui_Test_Window()
{
    //renders small text window element
    ImGui::Begin("New Window");
    ImGui::Text("Test");
    ImGui::End();
}
void Gui::swap_to_LockedCamera(){
    auto camera = Camera::get_Active_Camera();
    LockedCamera new_camera = LockedCamera(camera->get_Camera_Position(),glm::vec3(0.0f,0.0f,0.0f),camera->get_Camera_Speed());
}
void Gui::swap_to_FreeFlightCamera(){
    auto camera = Camera::get_Active_Camera();
    FreeFlightCamera new_camera = FreeFlightCamera(camera->get_Camera_Position(),glm::vec3(0.0f,0.0f,0.0f),camera->get_Camera_Speed());
}
/*
Display Window that shows Options the Control of the Camera:
-Camera Speed -> Slider
-Camera Movement:
    -Locked Camera-> Camera that is always locked to look at Planet, but can move around it. Zoom in/out.
    -Freeflight Camera-> Camera with completely free controls
*/
void Gui::imgui_Camera_Control_Window(bool* is_Locked_Camera,bool* is_Free_Camera, float* current_Speed)
{
    ImGui::Begin("Camera Controls");
        if(ImGui::Checkbox("Locked Camera",is_Locked_Camera)) {*is_Free_Camera = !(*is_Locked_Camera);}
        if(ImGui::Checkbox("Freeflight Camera",is_Free_Camera)) {*is_Locked_Camera = !(*is_Free_Camera);}
        ImGui::SliderFloat("Camera Speed",current_Speed,0.0f,1.0f);
    ImGui::End();
}


void Gui::imgui_Render()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Gui::imgui_Shutdown()
{
    //shutdown for ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}