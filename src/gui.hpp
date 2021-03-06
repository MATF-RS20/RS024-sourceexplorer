#ifndef GUI_HPP
#define GUI_HPP

#include <filesystem>
#include "TextEditor.h"
#include "clang_interface.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace fs = std::filesystem;

// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load
//  OpenGL function pointers. Helper libraries are often used for this purpose!
//  Here we are supporting a few common ones (gl3w, glew, glad). You may use
//  another loader/header of your choice (glext, glLoadGen, etc.), or chose to
//  manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>  // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>  // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>  // Initialize with gladLoadGL()
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to
// maximize ease of testing and compatibility with old VS compilers. To link
// with VS2010-era libraries, VS2015+ requires linking with
// legacy_stdio_definitions.lib, which we do using this pragma. Your own project
// should not be affected, as you are likely to link with a newer binary of GLFW
// that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && \
    !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

namespace gui {
class MainWindow {
  const char* glsl_version;
  GLFWwindow* window;
  bool err;

 public:
  GLFWwindow* Window() { return window; }

  MainWindow();

  ~MainWindow();
};

struct FileBrowser {
  std::vector<fs::path> files;
  bool warning = false;
  std::string new_name = "";
  std::string error_msg = "";

  void get_directory_files(const fs::path& pathname);
  void draw_filebrowser(const char* action, fs::path& filename, bool& write,
                        bool& is_clicked_OPEN);
};

class SourceCodePanel {
  ImGuiIO& io;
  TextEditor editor;
  FileBrowser file_browser;
  MainWindow& main_window;
  fs::path filename = "";
  fs::path file = fs::current_path();
  fs::path restore_filename = "";
  std::filesystem::path directory_of_last_opened_file;

  bool is_clicked_NEW = false;
  bool is_clicked_OPEN = false;
  bool bt_Save = false;
  bool unsaved = true;
  bool should_build_callgraph = false;
  bool* show_source_code_window;

 public:
  SourceCodePanel(ImGuiIO& io, MainWindow& main_window, bool* p_open)
      : io(io), main_window(main_window), show_source_code_window(p_open) {
    editor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
  }
  TextEditor& Editor() { return editor; }
  auto SecondsSinceLastTextChange() const {
    return editor.SecondsSinceLastTextChange();
  }
  std::filesystem::path DirectoryOfLastOpenedFile() const {
    return directory_of_last_opened_file;
  }
  auto IsTextChanged() const { return editor.IsTextChanged(); }
  bool ShouldBuildCallgraph() const { return should_build_callgraph; }
  void CallGraphBuilt() { should_build_callgraph = false; }
  const std::string SourceCode() const { return editor.GetText(); }
  void Draw();
};

class WindowsToggleMenu {
 private:
 public:
  bool show_source_code_window = true;
  bool show_callgraph_window = true;
  bool show_ast_dump_window = false;
  bool show_function_list_window = false;

  void Draw();
};

class FunctionListFilteringWindow {
 private:
  ImGuiTextFilter filter;
  const clang_interface::CallGraph::NodesList* functions{nullptr};
  clang_interface::FunctionDecl* last_clicked{nullptr};
  bool& p_open;

 public:
  explicit FunctionListFilteringWindow(bool& p_open) : p_open(p_open) {}
  clang_interface::FunctionDecl* LastClickedFunction() const {
    return last_clicked;
  }
  void SetFunctionsList(const clang_interface::CallGraph::NodesList* func) {
    functions = func;
    last_clicked = nullptr;
  }
  void Draw();
};

class FunctionASTDumpWindow {
 private:
  clang_interface::FunctionDecl* function{nullptr};
  bool& p_open;

 public:
  explicit FunctionASTDumpWindow(bool& p_open) : p_open(p_open) {}
  void SetFunction(clang_interface::FunctionDecl* func) { function = func; }
  void Clear() { function = nullptr; }
  void Draw();
};

};  // namespace gui

#endif  // GUI_HPP
