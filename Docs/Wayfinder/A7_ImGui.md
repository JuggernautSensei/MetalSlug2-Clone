# A7 - Dear ImGui (SDL3 + SDL_Renderer)
## vcpkg.json (replace nothing, append to "dependencies")
    { "name": "imgui", "features": [ "sdl3-binding", "sdl3-renderer-binding" ] }
- Feature names verified in the port at baseline e3ed4186 (imgui 1.91.9, sdl3 3.2.24): `sdl3-binding`, `sdl3-renderer-binding`.
- No baseline bump needed. vcpkg root: C:\vcpkg (vcxproj default; VCPKG_ROOT unset). Triplet x64-windows-static-md.
## Build wiring
- vcxproj uses vcpkg MSBuild integration (vcpkg.props/targets, VcpkgEnableManifest=true). SDL3 has NO explicit
  AdditionalDependencies/include entries; headers + libs are auto-added by VcpkgAutoLink. imgui is picked up the same way:
  no vcxproj edit needed. Include dir is vcpkg_installed\x64-windows-static-md\...\include (auto).
- Lib: imgui.lib (auto-linked). Headers: `#include <imgui.h>`, `<imgui_impl_sdl3.h>`,
  `<imgui_impl_sdlrenderer3.h>` (vcpkg installs backends at include root).
## Snippet (App.cpp / Renderer.cpp placement in comments)
    // Renderer::Init, after SDL_SetRenderDrawBlendMode:
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplSDL3_InitForSDLRenderer(m_pWindow, m_pRenderer);
    ImGui_ImplSDLRenderer3_Init(m_pRenderer);
    // App::PollEvents_, first line inside the while loop:
    ImGui_ImplSDL3_ProcessEvent(&msg);
    // frame start (after Renderer::BeginFrame):
    ImGui_ImplSDLRenderer3_NewFrame(); ImGui_ImplSDL3_NewFrame(); ImGui::NewFrame();
    ImGui::ShowDemoWindow();
    // frame end, in Renderer::EndFrame before SDL_RenderPresent:
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_pRenderer);
    // Renderer::Shutdown, before SDL_DestroyRenderer:
    ImGui_ImplSDLRenderer3_Shutdown(); ImGui_ImplSDL3_Shutdown(); ImGui::DestroyContext();
- Renderer uses SDL_SetRenderLogicalPresentation(INTEGER_SCALE): ImGui draws in logical space (tiny) and mouse needs
  SDL_ConvertEventToRenderCoordinates(m_pRenderer, &msg) before ProcessEvent. Consider SDL_SetRenderLogicalPresentation
  to DISABLED around the ImGui draw, then restore, if UI should be window-pixel sized.
## Verification
- `vcpkg install --dry-run --triplet x64-windows-static-md --vcpkg-root C:/vcpkg` (temp manifest copy, scratchpad):
  "imgui[core,sdl3-binding,sdl3-renderer-binding]:x64-windows-static-md@1.91.9" resolves with sdl3@3.2.24#1, no error.
- Signatures checked against imgui v1.91.9 raw headers (installed headers not present until install):
  ImGui_ImplSDL3_InitForSDLRenderer(SDL_Window*, SDL_Renderer*), ImGui_ImplSDL3_ProcessEvent(const SDL_Event*),
  ImGui_ImplSDL3_NewFrame/Shutdown, ImGui_ImplSDLRenderer3_Init(SDL_Renderer*)/NewFrame/RenderDrawData(ImDrawData*, SDL_Renderer*)/Shutdown.
