#include <array>
#include <vector>
#include <cmath>
#include <iostream>
#include <set>
#include <string_view>

#include <fmt/format.h>
#include <imgui.h>
#include <implot.h>

#include "render.hpp"

void WindowClass::Draw(std::string_view label)
{
    constexpr static auto window_flags =
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    constexpr static auto window_size = ImVec2(1280.0F, 720.0F);
    constexpr static auto window_pos = ImVec2(0.0F, 0.0F);

    ImGui::SetNextWindowSize(window_size);
    ImGui::SetNextWindowPos(window_pos);

    ImGui::Begin(label.data(), nullptr, window_flags);

    // DrawSelection();
    // DrawPlot();
    // Demo_RealtimePlots();
    TestingPlot();

    ImGui::End();
}

void WindowClass::DrawSelection()
{
    for (const auto func_name : functionNames)
    {
        const auto curr_function = functionNameMapping(func_name);
        auto selected = selectedFunctions.contains(curr_function);

        if (ImGui::Checkbox(func_name.data(), &selected))
        {
            if (selected)
                selectedFunctions.insert(curr_function);
            else
                selectedFunctions.erase(curr_function);
        }
    }
}

void WindowClass::DrawPlot()
{
    static constexpr auto num_points = 10'000;
    static constexpr auto x_min = -100.0;
    static constexpr auto x_max = 100.0;
    static const auto x_step = (std::abs(x_max) + std::abs(x_min)) / num_points;

    static auto xs = std::array<double, num_points>{};
    static auto ys = std::array<double, num_points>{};

    if (selectedFunctions.size() == 0 ||
        (selectedFunctions.size() == 1 &&
         *selectedFunctions.begin() == Function::NONE))
    {
        ImPlot::BeginPlot("###plot", ImVec2(-1.0F, -1.0F), ImPlotFlags_NoTitle);
        ImPlot::EndPlot();
        return;
    }

    ImPlot::BeginPlot("###plot", ImVec2(-1.0F, -1.0F), ImPlotFlags_NoTitle);

    for (const auto &function : selectedFunctions)
    {
        auto x = x_min;
        for (std::size_t i = 0; i < num_points; ++i)
        {
            xs[i] = x;
            ys[i] = evaluateFunction(function, x);
            x += x_step;
        }

        const auto plot_label =
            fmt::format("##function{}", static_cast<int>(function));
        ImPlot::PlotLine(plot_label.data(), xs.data(), ys.data(), num_points);
    }

    ImPlot::EndPlot();
}

void WindowClass::Demo_RealtimePlots() {
    // utility structure for realtime plot
    struct ScrollingBuffer {
        int MaxSize;
        int Offset;
        ImVector<ImVec2> Data;
        ScrollingBuffer(int max_size = 200000) {
            MaxSize = max_size;
            Offset  = 0;
            Data.reserve(MaxSize);
        }
        void AddPoint(double x, float y) {
            if (Data.size() < MaxSize)
                Data.push_back(ImVec2(x,y));
            else {
                Data[Offset] = ImVec2(x,y);
                Offset =  (Offset + 1) % MaxSize;
            }
        }
        void Erase() {
            if (Data.size() > 0) {
                Data.shrink(0);
                Offset  = 0;
            }
        }
    };

    // utility structure for realtime plot
    struct RollingBuffer {
        float Span;
        ImVector<ImVec2> Data;
        RollingBuffer() {
            Span = 10.0f;
            Data.reserve(200000);
        }
        void AddPoint(float x, float y) {
            float xmod = fmodf(x, Span);
            if (!Data.empty() && xmod < Data.back().x)
                Data.shrink(0);
            Data.push_back(ImVec2(xmod, y));
        }
    };

    ImGui::BulletText("Move your mouse to change the data!");
    ImGui::BulletText("This example assumes 60 FPS. Higher FPS requires larger buffer size.");
    static ScrollingBuffer sdata1, sdata2;
    static RollingBuffer   rdata1, rdata2;
    ImVec2 mouse = ImGui::GetMousePos();
    static float time_start = (float)std::time(nullptr);
    static float t = (float)std::time(nullptr);
    t += ImGui::GetIO().DeltaTime;
    ImGui::BulletText(std::to_string(t).c_str());
    sdata1.AddPoint(t, mouse.x * 0.0005f);
    rdata1.AddPoint(t, mouse.x * 0.0005f);
    sdata2.AddPoint(t, mouse.y * 0.0005f);
    rdata2.AddPoint(t, mouse.y * 0.0005f);

    ImGui::Checkbox("Local Time",&ImPlot::GetStyle().UseLocalTime);
    ImGui::SameLine();
    ImGui::Checkbox("ISO 8601",&ImPlot::GetStyle().UseISO8601);
    ImGui::SameLine();
    ImGui::Checkbox("24 Hour Clock",&ImPlot::GetStyle().Use24HourClock);

    static float history = 10.0f;
    ImGui::SliderFloat("History",&history,1,30,"%.1f s");
    rdata1.Span = history;
    rdata2.Span = history;

    static ImPlotAxisFlags flags = NULL;

    if (ImPlot::BeginPlot("##Scrolling", ImVec2(-1,150))) {
        ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
        ImPlot::SetupAxisLimits(ImAxis_X1,t - history, t, ImGuiCond_Always);
        ImPlot::SetupAxisLimits(ImAxis_Y1,0,1);
        ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL,0.5f);
        ImPlot::PlotShaded("Mouse X", &sdata1.Data[0].x, &sdata1.Data[0].y, sdata1.Data.size(), -INFINITY, 0, sdata1.Offset, 2 * sizeof(float));
        ImPlot::PlotLine("Mouse Y", &sdata2.Data[0].x, &sdata2.Data[0].y, sdata2.Data.size(), 0, sdata2.Offset, 2*sizeof(float));
        ImPlot::EndPlot();
    }
    if (ImPlot::BeginPlot("##Rolling", ImVec2(-1,150))) {
        ImPlot::SetupAxes(nullptr, nullptr, flags, flags);
        ImPlot::SetupAxisLimits(ImAxis_X1,0,history, ImGuiCond_Always);
        ImPlot::SetupAxisLimits(ImAxis_Y1,0,1);
        ImPlot::PlotLine("Mouse X", &rdata1.Data[0].x, &rdata1.Data[0].y, rdata1.Data.size(), 0, 0, 2 * sizeof(float));
        ImPlot::PlotLine("Mouse Y", &rdata2.Data[0].x, &rdata2.Data[0].y, rdata2.Data.size(), 0, 0, 2 * sizeof(float));
        ImPlot::EndPlot();
    }
}

void WindowClass::TestingPlot() {
    // std::vector<double> t, y;

    // static const time_t plot_start_time = std::time(nullptr);

    // if (t.empty()) {
    //     for (int i = 0; i < 1000; ++i) {
    //         t.push_back((double) plot_start_time + i);
    //         y.push_back(std::sin((double)i / 10.0));
    //     }
    // }

    // ImPlot::GetStyle().UseLocalTime = true;
    // if (ImPlot::BeginPlot("My Plot", "t", "y")) {
    //     ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
    //     ImPlot::PlotLine("sin(x)", t.data(), y.data(), t.size());
    //     ImPlot::EndPlot();
    // }

    static std::vector<double> t, y;
    static const time_t plot_start_time = std::time(nullptr);
    static const time_t max_time_range = 100;
    const time_t current_time = std::time(nullptr);

    // Create a sin-wave as time progresses (once every second)
    if (t.empty()) {
        t.push_back(current_time);
        y.push_back(std::sin(current_time / 10.0));
    }

    if (current_time != t.back()) {
        t.push_back(current_time);
        y.push_back(std::sin(current_time / 10.0));
    }

    // Remove old data
    if (!t.empty() && t.front() - t.back() > max_time_range) {
        t.erase(t.begin());
        y.erase(y.begin());
    }

    ImGui::BulletText("Current plot x-axis lower limit: %ld", current_time - max_time_range);
    ImGui::BulletText("Current plot x-axis upper limit: %ld", current_time);

    ImGui::BulletText("Current size of t: %ld", t.size());

    static bool set_axis_to_current_time = false;
    ImGui::Checkbox("Real-time",&set_axis_to_current_time);

    // Plot the data in a time-series plot
    if (ImPlot::BeginPlot("My Plot", "t", "y")) {
        ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
        if (set_axis_to_current_time) {
            ImPlot::SetupAxisLimits(ImAxis_X1, current_time - max_time_range, current_time, ImGuiCond_Always);
        }
        else {
            ImPlot::SetupAxisLimits(ImAxis_X1, current_time - max_time_range, current_time);
        }
        ImPlot::PlotLine("sin(x)", t.data(), y.data(), t.size());

        // Display the current plot limits
        ImGui::BulletText("X-axis range: [%ld, %ld]", (int)(ImPlot::GetPlotLimits().X.Min), (int)(ImPlot::GetPlotLimits().X.Max));
        ImGui::BulletText("Y-axis range: [%ld, %ld]", (int)(ImPlot::GetPlotLimits().Y.Min), (int)(ImPlot::GetPlotLimits().Y.Max));

        ImPlot::EndPlot();
    }


}

WindowClass::Function WindowClass::functionNameMapping(
    std::string_view function_name)
{
    if (std::string_view{"sin(x)"} == function_name)
        return WindowClass::Function::SIN;

    if (std::string_view{"cos(x)"} == function_name)
        return WindowClass::Function::COS;

    return WindowClass::Function::NONE;
}

double WindowClass::evaluateFunction(const Function function, const double x)
{
    switch (function)
    {
    case Function::SIN:
    {
        return std::sin(x);
    }
    case Function::COS:
    {
        return std::cos(x);
    }
    case Function::NONE:
    default:
    {
        return 0.0;
    }
    }
}

void render(WindowClass &window_obj)
{
    window_obj.Draw("Calculator Tool");
}
