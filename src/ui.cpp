#include "ui.h"
#include "shader.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

static bool show_help = false;

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
  (void)scancode;
  (void)window;
  if (action != GLFW_PRESS) return;

  // Toggle help with '?' (Shift + /)
  if (key == GLFW_KEY_SLASH && (mods & GLFW_MOD_SHIFT)) {
    show_help = !show_help;
  }
  // Switch shaders with left/right
  if (key == GLFW_KEY_LEFT) {
    shader_prev();
  }
  if (key == GLFW_KEY_RIGHT) {
    shader_next();
  }
}

void ui_init(GLFWwindow *window) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330");

  glfwSetKeyCallback(window, key_callback);
}

void ui_render(void) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  if (show_help) {
    ImGuiIO &io = ImGui::GetIO();
    ImVec2 window_size(300, 80);
    ImVec2 window_pos((io.DisplaySize.x - window_size.x) * 0.5f,
                      io.DisplaySize.y - window_size.y - 20);

    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar;

    ImGui::Begin("Controls", nullptr, flags);
    ImGui::Text("Shader: %s", shader_get_current_name());
    ImGui::Separator();
    ImGui::Text("<Left/Right> Change Shader");
    ImGui::Text("<Up/Down> Effect Mod");
    ImGui::End();
  }

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ui_cleanup(void) {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}
