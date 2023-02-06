#include "view/block/tab_viewer.h"

#include "util/logger.h"
#include "view/block/tab_item/audio_equalizer.h"
#include "view/block/tab_item/spectrum_visualizer.h"

namespace interface {

/* ********************************************************************************************** */

TabViewer::TabViewer(const std::shared_ptr<EventDispatcher>& dispatcher)
    : Block{dispatcher, Identifier::TabViewer, interface::Size{.width = 0, .height = 0}},
      btn_help_{nullptr},
      btn_exit_{nullptr},
      active_{View::Visualizer},
      views_{} {
  // Initialize window buttons
  btn_help_ = Button::make_button_for_window(std::string("F1:help"), [&]() {
    LOG("Handle left click mouse event on Help button");
    auto dispatcher = dispatcher_.lock();
    if (dispatcher) {
      auto event = interface::CustomEvent::ShowHelper();
      dispatcher->SendEvent(event);
    }
  });

  btn_exit_ = Button::make_button_for_window(std::string("X"), [&]() {
    LOG("Handle left click mouse event on Exit button");
    auto dispatcher = dispatcher_.lock();
    if (dispatcher) {
      auto event = interface::CustomEvent::Exit();
      dispatcher->SendEvent(event);
    }
  });

  // Add tab views
  views_[View::Visualizer] = Tab{
      .key = std::string{"1"},
      .button = Button::make_button_for_window(
          std::string{"1:visualizer"},
          [&]() {
            LOG("Handle left click mouse event on Tab button for visualizer");
            active_ = View::Visualizer;
          },
          Button::Delimiters{"", ""}),
      .item = std::make_unique<SpectrumVisualizer>(dispatcher),
  };

  views_[View::Equalizer] = Tab{
      .key = std::string{"2"},
      .button = Button::make_button_for_window(
          std::string{"2:equalizer"},
          [&]() {
            LOG("Handle left click mouse event on Tab button for equalizer");
            active_ = View::Equalizer;
          },
          Button::Delimiters{"", ""}),
      .item = std::make_unique<AudioEqualizer>(dispatcher),
  };
}

/* ********************************************************************************************** */

ftxui::Element TabViewer::Render() {
  auto get_decorator_for = [&](const View& v) {
    return (active_ == v) ? ftxui::nothing : ftxui::color(ftxui::Color::GrayDark);
  };

  auto btn_visualizer = views_[View::Visualizer].button->Render();
  auto btn_equalizer = views_[View::Equalizer].button->Render();

  ftxui::Element title_border = ftxui::hbox({
      ftxui::text(" "),
      btn_visualizer | get_decorator_for(View::Visualizer),
      ftxui::text(" "),
      btn_equalizer | get_decorator_for(View::Equalizer),
      ftxui::text(" "),
      ftxui::filler(),
      btn_help_->Render(),
      ftxui::text(" ") | ftxui::border,  // dummy space between buttons
      btn_exit_->Render(),
  });

  ftxui::Element view = views_[active_].item->Render();

  return ftxui::window(title_border, view | ftxui::yflex);
}

/* ********************************************************************************************** */

bool TabViewer::OnEvent(ftxui::Event event) {
  if (event.is_mouse()) return OnMouseEvent(event);

  // Check if event is equal to a registered keybinding for any of the tab items
  auto found = std::find_if(views_.begin(), views_.end(),
                            [event](auto&& t) { return t.second.key == event.character(); });

  // Found some mapped keybinding, now check if this is already the active view
  if (found != views_.end() && active_ != found->first) {
    active_ = found->first;
    return true;
  }

  // Otherwise, let item handle it
  return views_[active_].item->OnEvent(event);
}

/* ********************************************************************************************** */

bool TabViewer::OnCustomEvent(const CustomEvent& event) {
  return views_[active_].item->OnCustomEvent(event);
}

/* ********************************************************************************************** */

bool TabViewer::OnMouseEvent(ftxui::Event event) {
  if (btn_help_->OnEvent(event)) return true;

  if (btn_exit_->OnEvent(event)) return true;

  for (auto& view : views_) {
    if (view.second.button->OnEvent(event)) return true;
  }

  return views_[active_].item->OnMouseEvent(event);
}

}  // namespace interface
