#include <algorithm>

#include "GUI.h"

#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "glm/gtc/type_ptr.hpp"

void GUI::keyCallback(int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_P) m_is_panel_visable = !m_is_panel_visable;
        if (key == GLFW_KEY_R) resetCamera();
    }
}

void GUI::mouseButtonCallback(int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) m_mouse_right_button = true;
        else if (action == GLFW_RELEASE) m_mouse_right_button = false;
    }
}

void GUI::cursorPosCallback(double xpos, double ypos) {
    xpos += 0.5;
    ypos += 0.5;
    xpos *= (m_window_dimentions.x != 0) ? 1.0 / double(m_window_dimentions.x) : 0.0;
    ypos *= (m_window_dimentions.y != 0) ? 1.0 / double(m_window_dimentions.y) : 0.0;
    xpos = 2.0 * xpos - 1.0;
    ypos = -2.0 * ypos + 1.0;
    glm::vec2 new_mouse_pos{xpos, ypos};
    if (m_mouse_right_button) {
        glm::vec2 delta_mouse_pos = m_mouse_pos - new_mouse_pos;
        glm::vec2 delta_camera_pos{
            m_scale * m_aspect * delta_mouse_pos.x, m_scale * delta_mouse_pos.y};
        if (using_3d) {
            m_camera_3d.incrementTheta(delta_mouse_pos.y * 3.f);
            m_camera_3d.incrementPhi(-delta_mouse_pos.x * 3.f);
        } else {
            m_camera.incrementTarget(delta_camera_pos);
        }
    }
    m_mouse_pos = new_mouse_pos;
}

void GUI::scrollCallback(double xoffset, double yoffset) {
    if (using_3d) m_camera_3d.incrementR(m_zoom_rate * yoffset);
    else m_scale -= m_zoom_rate * float(yoffset);
}

void GUI::windowSizeCallback(int width, int height) {
    m_window_dimentions = glm::uvec2(width, height);
    m_aspect            = (height != 0) ? float(width) / float(height) : 1.f;
    resetViewPort();
}

glm::mat4 GUI::getProjectionMatrix() {
    if (using_3d) return glm::perspective(glm::radians(45.0f), m_aspect, 0.01f, 1000.f);
    return glm::ortho(-m_scale * m_aspect, m_scale * m_aspect, -m_scale, m_scale, 1.f, -1.f);
}

glm::mat4 GUI::getViewMatrix() {
    if (using_3d) return m_camera_3d.getView();
    return m_camera.getView();
}

glm::vec3 GUI::getViewPosition() {
    if (using_3d) return m_camera_3d.getPos();
    return m_camera.getPos();
}

void GUI::resetViewPort() {
    CallbackInterface::windowSizeCallback(m_window_dimentions.x, m_window_dimentions.y);
}

void GUI::resetCamera() {
    m_camera.setTarget(glm::vec2(0.0));
    m_scale = 1.f;
    m_camera_3d.setR(3.f);
    m_camera_3d.setPhi(0.f);
    m_camera_3d.incrementTheta(0.f);
}

glm::vec4 GUI::getClearColor() { return {m_clear_color.x, m_clear_color.y, m_clear_color.z, 1.f}; }

glm::vec2 GUI::getMousePos() { return m_mouse_pos; }

glm::ivec2 GUI::getProbabilitySurfaceRoot() { return m_ps_root; }

glm::ivec2 GUI::getProbabilitySurfaceDim() { return m_ps_dim; }

bool GUI::isQuadsVisable() { return m_quads_visable; }

glm::ivec2 GUI::getCloudQuadMinMax() { return m_cloud_cull_min_max; }

void GUI::draw() {
    using namespace ImGui;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    NewFrame();
    if (m_is_panel_visable && Begin("panel", &m_is_panel_visable, ImGuiWindowFlags_MenuBar)) {
        Separator();
        Text(
            "Application average %.3f ms/frame (%.1f FPS)",
            1000.0f / GetIO().Framerate,
            GetIO().Framerate
        );
        ColorEdit3("Clear color", (float *)&m_clear_color);
        DragFloat("Zoom Factor", &m_zoom_rate, 0.001f, 0.f, 1.f);
        DragFloat("DUMMY FLOAT", &m_dummy_float, 0.01f, 0.f, 10.f);
        Separator();
        if (m_registered_views.size() > 0) {
            if (ImGui::BeginCombo("View", m_registered_views[m_view_selected].c_str())) {
                for (int n = 0; n < m_registered_views.size(); n++) {
                    bool is_selected = m_view_selected == n;
                    if (ImGui::Selectable(m_registered_views[n].c_str(), is_selected))
                        m_view_selected = n;
                    if (is_selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        }
        static glm::ivec2 hist_bounds = {0, m_image_histogram_summary.size() - 1};
        SliderInt2(
            "Histogram Bounds", glm::value_ptr(hist_bounds), 0, m_image_histogram_summary.size() - 1
        );
        hist_bounds.y = std::max(hist_bounds.y, 1);
        hist_bounds.x = std::min(hist_bounds.x, hist_bounds.y - 1);
        PlotHistogram(
            "Pixel Distrabution",
            m_image_histogram_summary.data(),
            hist_bounds.y - hist_bounds.x,
            hist_bounds.x,
            "Test",
            0.f,
            1.f,
            ImVec2(0, 200),
            sizeof(float)
        );
        Checkbox("View Cloud Quads", &m_quads_visable);
        DragInt2(
            "Cloud Quad Cull Bounds",
            glm::value_ptr(m_cloud_cull_min_max),
            1,
            0,
            std::numeric_limits<int>::max()
        );
        DragInt2("Probability Surface Root", glm::value_ptr(m_ps_root), 1, -1024, 1024);
        DragInt2("Probability Surface Dim", glm::value_ptr(m_ps_dim), 1, -1024, 1024);
        Separator();
        End();
    }
    // EndFrame();
    Render();
    ImGui_ImplOpenGL3_RenderDrawData(GetDrawData());
}

bool GUI::registerView(std::string input) {
    if (existsInViews(input) || input.empty()) return false;
    m_registered_views.push_back(input);
    refreshViewHeader();
    return true;
}

bool GUI::deregisterView(std::string input) {
    if (!existsInViews(input) || input.empty()) return false;
    m_registered_views.erase(std::find(m_registered_views.begin(), m_registered_views.end(), input)
    );
    refreshViewHeader();
    return true;
}

void GUI::clearViews() {
    m_registered_views.clear();
    refreshViewHeader();
}

std::string GUI::currentView() {
    if (m_view_selected < 0) return std::string();
    return m_registered_views[m_view_selected];
}

void GUI::set3DView(bool v) {
    if (using_3d != v) resetCamera();
    using_3d = v;
}

void GUI::setImageHistogram(float *data, size_t size) {
    m_image_histogram_summary = std::vector<float>(256, 0.f);
    for (int i = 0; i < size; i++) {
        size_t index = std::min(size_t(data[i] * 256), size_t(255));
        m_image_histogram_summary[index] += 1.f;
    }
    for (auto &buck : m_image_histogram_summary) {
        buck /= float(size);
    }
}

void GUI::refreshViewHeader() {
    if (m_registered_views.size() > 0) {
        std::string header_temp;
        for (auto &s : m_registered_views) {
            header_temp.append(s);
            header_temp.append("\0");
        }
        m_registered_views_header = std::make_shared<const char *>(header_temp.c_str());
        m_view_selected
            = std::min(std::max(m_view_selected, 0), int(m_registered_views.size()) - 1);
    } else {
        m_registered_views_header = std::make_shared<const char *>();
        m_view_selected           = -1;
    }
}

bool GUI::existsInViews(std::string input) {
    return std::find(m_registered_views.begin(), m_registered_views.end(), input)
        != m_registered_views.end();
}