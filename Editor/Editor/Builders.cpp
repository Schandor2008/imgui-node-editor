#include "Builders.h"
#include "ImGuiInterop.h"
#include "Application/imgui_impl_dx11.h"


//------------------------------------------------------------------------------
namespace ed   = ax::Editor;
namespace util = ax::Editor::Utilities;

util::BluepringNodeBuilder::BluepringNodeBuilder(ImTextureID texture, int textureWidth, int textureHeight):
    HeaderTextureId(texture),
    HeaderTextureWidth(textureWidth),
    HeaderTextureHeight(textureHeight),
    CurrentNodeId(0),
    CurrentStage(Stage::Invalid)
{
}

void util::BluepringNodeBuilder::Begin(int id)
{
    ed::PushStyleVar(StyleVar_NodePadding, ImVec4(8, 4, 8, 8));

    ed::BeginNode(id);

    ImGui::PushID(id);
    CurrentNodeId = id;

    SetStage(Stage::Begin);
}

void util::BluepringNodeBuilder::End()
{
    using namespace ImGuiInterop;

    SetStage(Stage::End);

    ed::EndNode();

    if (ImGui::IsItemVisible())
    {
        auto alpha = static_cast<int>(255 * ImGui::GetStyle().Alpha);

        auto drawList = ed::GetNodeBackgroundDrawList(CurrentNodeId);

        const auto halfBorderWidth = ed::GetStyle().NodeBorderWidth * 0.5f;

        auto headerColor = IM_COL32(0, 0, 0, alpha) | (HeaderColor & IM_COL32(255, 255, 255, 0));
        if (!HeaderRect.is_empty() && HeaderTextureId)
        {
            const auto uv = ImVec2(
                HeaderRect.w / (float)(4.0f * HeaderTextureWidth),
                HeaderRect.h / (float)(4.0f * HeaderTextureHeight));

            drawList->AddImage(HeaderTextureId,
                to_imvec(HeaderRect.top_left())     - ImVec2(8 - halfBorderWidth, 4 - halfBorderWidth),
                to_imvec(HeaderRect.bottom_right()) + ImVec2(8 - halfBorderWidth, 0),
                ImVec2(0.0f, 0.0f), uv,
                headerColor, GetStyle().NodeRounding, 1 | 2);
        }

        auto headerSeparatorRect = ax::rect(HeaderRect.bottom_left(), ContentRect.top_right());
        auto footerSeparatorRect      = ax::rect(ContentRect.bottom_left(), NodeRect.bottom_right());
        auto contentWithSeparatorRect = ax::rect::make_union(headerSeparatorRect, footerSeparatorRect);

        if (!headerSeparatorRect.is_empty())
        {
            drawList->AddLine(
                to_imvec(headerSeparatorRect.top_left())  + ImVec2(-(8 - halfBorderWidth), -1),
                to_imvec(headerSeparatorRect.top_right()) + ImVec2( (8 - halfBorderWidth), -1),
                ImColor(255, 255, 255, 96 * alpha / (3 * 255)), 1.0f);
        }

        //drawList->AddRect(to_imvec(NodeRect.top_left()), to_imvec(NodeRect.bottom_right()), IM_COL32(255, 0, 0, 255));
        //drawList->AddRect(to_imvec(HeaderRect.top_left()), to_imvec(HeaderRect.bottom_right()), IM_COL32(0, 255, 0, 255));
        //drawList->AddRect(to_imvec(ContentRect.top_left()), to_imvec(ContentRect.bottom_right()), IM_COL32(0, 0, 255, 255));
    }

    CurrentNodeId = 0;

    ImGui::PopID();

    ed::PopStyleVar();

    SetStage(Stage::Invalid);
}

void util::BluepringNodeBuilder::Header(const ImVec4& color)
{
    HeaderColor = ImColor(color);
    SetStage(Stage::Header);
}

void util::BluepringNodeBuilder::EndHeader()
{
    SetStage(Stage::Content);
}

void util::BluepringNodeBuilder::Input(int id)
{
    if (CurrentStage == Stage::Begin)
        SetStage(Stage::Content);

    const auto applyPadding = (CurrentStage == Stage::Input);

    SetStage(Stage::Input);

    if (applyPadding)
        ImGui::Spring(0);

    Pin(id, PinKind::Target);

    ImGui::BeginHorizontal(id);
}

void util::BluepringNodeBuilder::EndInput()
{
    ImGui::EndHorizontal();

    EndPin();
}

void util::BluepringNodeBuilder::Output(int id)
{
    if (CurrentStage == Stage::Begin)
        SetStage(Stage::Content);

    if (CurrentStage == Stage::Begin)
        SetStage(Stage::Input);

    const auto applyPadding = (CurrentStage == Stage::Output);

    SetStage(Stage::Output);

    if (applyPadding)
        ImGui::Spring(0);

    Pin(id, PinKind::Source);

    ImGui::BeginHorizontal(id);
}

void util::BluepringNodeBuilder::EndOutput()
{
    ImGui::EndHorizontal();

    EndPin();
}

bool util::BluepringNodeBuilder::SetStage(Stage stage)
{
    using namespace ImGuiInterop;

    if (stage == CurrentStage)
        return false;

    auto oldStage = CurrentStage;
    CurrentStage = stage;

    ImVec2 cursor;
    switch (oldStage)
    {
        case Stage::Begin:
            break;

        case Stage::Header:
            ImGui::EndHorizontal();
            HeaderRect = ImGui_GetItemRect();

            // spacing between header and content
            ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.y * 2.0f);

            break;

        case Stage::Content:
            break;

        case Stage::Input:
            ImGui::Spring(1, 0);
            ImGui::EndVertical();
            break;

        case Stage::Output:
            ImGui::Spring(1);
            ImGui::EndVertical();
            break;

        case Stage::End:
            break;
    }

    switch (stage)
    {
        case Stage::Begin:
            ImGui::BeginVertical("node");
            break;

        case Stage::Header:
            ImGui::BeginHorizontal("header");
            break;

        case Stage::Content:
            ImGui::BeginHorizontal("content");
            ImGui::Spring(0, 0);
            break;

        case Stage::Input:
            ImGui::BeginVertical("inputs", ImVec2(0, 0), 0.0f);
            break;

        case Stage::Output:
            ImGui::Spring(1);
            ImGui::BeginVertical("outputs", ImVec2(0, 0), 1.0f);
            break;

        case Stage::End:
            if (oldStage == Stage::Input)
                ImGui::Spring(1, 0);
            ImGui::EndHorizontal();
            ContentRect = ImGui_GetItemRect();

            //ImGui::Spring(0);
            ImGui::EndVertical();
            NodeRect = ImGui_GetItemRect();
            break;
    }

    return true;
}

void util::BluepringNodeBuilder::Pin(int id, ed::PinKind kind)
{
    const ImVec2 pivot = kind == ed::PinKind::Source ? ImVec2(1.0f, 0.5f) : ImVec2(0.0f, 0.5f);

    ed::BeginPin(id, kind, pivot);
}

void util::BluepringNodeBuilder::EndPin()
{
    ed::EndPin();
}