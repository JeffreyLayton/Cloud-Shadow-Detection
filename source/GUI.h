#pragma once

#include <glad/glad.h>

#include <string>
#include <vector>

#include <imgui.h>

#include "boilerplate/Camera.h"
#include "boilerplate/Window.h"
#include "glm/glm.hpp"

class GUI : public CallbackInterface {
  public:
    GUI(glm::uvec2 window_dimentions)
        : using_3d(false)
        , m_camera({0.f, 0.f})
        , m_camera_3d(0.f, 0.f, 3.f)
        , m_mouse_right_button(false)
        , m_mouse_pos({0.f, 0.f})
        , m_window_dimentions(window_dimentions)
        , m_aspect(
              (window_dimentions.y != 0) ? float(window_dimentions.x) / float(window_dimentions.y)
                                         : 1.f
          )
        , m_scale(1.f)
        , m_zoom_rate(0.1f)
        , m_is_panel_visable(true)
        , m_clear_color(0.3f, 0.3f, 1.f, 1.f)
        , m_registered_views()
        , m_registered_views_header(std::make_shared<const char *>())
        , m_view_selected(-1)
        , m_dummy_float(0.f)
        , m_quads_visable(false)
        , m_cloud_cull_min_max(0, std::numeric_limits<int>::max())
        , m_ps_root(-1, -1)
        , m_ps_dim(258, 258)
        , m_image_histogram_summary(256, 0.f) {}

    void keyCallback(int key, int scancode, int action, int mods);
    void mouseButtonCallback(int button, int action, int mods);
    void cursorPosCallback(double xpos, double ypos);
    void scrollCallback(double xoffset, double yoffset);
    void windowSizeCallback(int width, int height);

    void resetViewPort();
    void resetCamera();

    glm::mat4 getProjectionMatrix();
    glm::mat4 getViewMatrix();
    glm::vec3 getViewPosition();
    glm::vec4 getClearColor();
    glm::vec2 getMousePos();
    glm::ivec2 getProbabilitySurfaceRoot();
    glm::ivec2 getProbabilitySurfaceDim();
    bool isQuadsVisable();
    glm::ivec2 getCloudQuadMinMax();

    void draw();

    bool registerView(std::string input);
    bool deregisterView(std::string input);
    void clearViews();
    std::string currentView();

    void set3DView(bool v);
    void setImageHistogram(float *data, size_t size);

    float getDummyFloat() { return m_dummy_float; }

  private:
    float m_dummy_float;
    // --- Callback --- //
    bool using_3d;
    Camera2D m_camera;
    CameraTurnTable m_camera_3d;
    bool m_mouse_right_button;
    glm::vec2 m_mouse_pos;
    glm::uvec2 m_window_dimentions;
    float m_aspect;
    float m_scale;
    float m_zoom_rate;
    // --- Panel --- //
    bool m_is_panel_visable;
    ImVec4 m_clear_color;
    std::vector<std::string> m_registered_views;
    std::shared_ptr<const char *> m_registered_views_header;
    int m_view_selected;
    void refreshViewHeader();
    bool existsInViews(std::string input);
    bool m_quads_visable;
    glm::ivec2 m_cloud_cull_min_max;
    glm::ivec2 m_ps_root;
    glm::ivec2 m_ps_dim;

    std::vector<float> m_image_histogram_summary;
};
