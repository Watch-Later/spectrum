#include "view/block/tab_viewer.h"

#include "util/logger.h"
#include "view/block/tab_item/audio_equalizer.h"
#include "view/block/tab_item/song_lyric.h"
#include "view/block/tab_item/spectrum_visualizer.h"

namespace interface {

TabViewer::TabViewer(const std::shared_ptr<EventDispatcher>& dispatcher)
    : Block{dispatcher, model::BlockIdentifier::TabViewer,
            interface::Size{.width = 0, .height = 0}} {
  // Initialize window buttons
  CreateButtons();

  // Add tab views
  CreateViews(dispatcher);
}

/* ********************************************************************************************** */

ftxui::Element TabViewer::Render() {
  auto get_decorator_for = [&](const View& v) {
    return (active_ == v) ? GetTitleDecorator() : ftxui::color(ftxui::Color::GrayDark);
  };

  auto btn_visualizer = views_[View::Visualizer].button->Render();
  auto btn_equalizer = views_[View::Equalizer].button->Render();
  auto btn_lyric = views_[View::Lyric].button->Render();

  ftxui::Element title_border = ftxui::hbox({
      btn_visualizer | get_decorator_for(View::Visualizer),
      btn_equalizer | get_decorator_for(View::Equalizer),
      btn_lyric | get_decorator_for(View::Lyric),
      ftxui::filler(),
      btn_help_->Render(),
      ftxui::text(" ") | ftxui::border,  // dummy space between buttons
      btn_exit_->Render(),
  });

  ftxui::Element view = active()->Render();

  return ftxui::window(title_border, view | ftxui::yflex);
}

/* ********************************************************************************************** */

bool TabViewer::OnEvent(ftxui::Event event) {
  if (event.is_mouse()) return OnMouseEvent(event);

  // Check if event is equal to a registered keybinding for any of the tab items.
  if (auto found =
          std::find_if(views_.begin(), views_.end(),
                       [&event](const auto& t) { return t.second.key == event.character(); });
      found != views_.end()) {
    auto dispatcher = GetDispatcher();

    // Set this block as active (focused)
    auto event_focus = interface::CustomEvent::SetFocused(GetId());
    dispatcher->SendEvent(event_focus);

    // Update active tab
    active_ = found->first;

    return true;
  }

  // If block is not focused, do not even try to handle event
  if (!IsFocused()) {
    return false;
  }

  // Otherwise, let item handle it
  return active()->OnEvent(event);
}

/* ********************************************************************************************** */

bool TabViewer::OnCustomEvent(const CustomEvent& event) {
  // Even if TabItem::SongLyrics is not active, force it to process these events
  // By doing this, it makes possible to fetch lyrics on background
  if ((event == CustomEvent::Identifier::ClearSongInfo ||
       event == CustomEvent::Identifier::UpdateSongInfo) &&
      active_ != View::Lyric) {
    views_[View::Lyric].item->OnCustomEvent(event);
  }

  return active()->OnCustomEvent(event);
}

/* ********************************************************************************************** */

bool TabViewer::OnMouseEvent(ftxui::Event event) {
  if (btn_help_->OnMouseEvent(event)) return true;

  if (btn_exit_->OnMouseEvent(event)) return true;

  for (const auto& [id, item] : views_) {
    if (item.button->OnMouseEvent(event)) {
      return true;
    }
  }

  return active()->OnMouseEvent(event);
}

/* ********************************************************************************************** */

void TabViewer::CreateButtons() {
  btn_help_ = Button::make_button_for_window(std::string("F1:help"), [this]() {
    auto disp = GetDispatcher();

    LOG("Handle left click mouse event on Help button");
    auto event = interface::CustomEvent::ShowHelper();
    disp->SendEvent(event);

    return true;
  });

  btn_exit_ = Button::make_button_for_window(std::string("X"), [this]() {
    auto disp = GetDispatcher();

    LOG("Handle left click mouse event on Exit button");
    auto event = interface::CustomEvent::Exit();
    disp->SendEvent(event);

    return true;
  });
}

/* ********************************************************************************************** */

void TabViewer::CreateViews(const std::shared_ptr<EventDispatcher>& dispatcher) {
  views_[View::Visualizer] = Tab{
      .key = "1",
      .button = Button::make_button_for_window(
          std::string{"1:visualizer"},
          [this]() {
            LOG("Handle left click mouse event on Tab button for visualizer");
            active_ = View::Visualizer;

            // Send event to set focus on this block
            AskForFocus();

            return true;
          },
          Button::Delimiters{" ", " "}),
      .item = std::make_unique<SpectrumVisualizer>(GetId(), dispatcher),
  };

  views_[View::Equalizer] = Tab{
      .key = "2",
      .button = Button::make_button_for_window(
          std::string{"2:equalizer"},
          [this]() {
            LOG("Handle left click mouse event on Tab button for equalizer");
            active_ = View::Equalizer;

            // Send event to set focus on this block
            AskForFocus();

            return true;
          },
          Button::Delimiters{" ", " "}),
      .item = std::make_unique<AudioEqualizer>(GetId(), dispatcher),
  };

  views_[View::Lyric] = Tab{
      .key = "3",
      .button = Button::make_button_for_window(
          std::string{"3:lyric"},
          [this]() {
            LOG("Handle left click mouse event on Tab button for lyric");
            active_ = View::Lyric;

            // Send event to set focus on this block
            AskForFocus();

            return true;
          },
          Button::Delimiters{" ", " "}),
      .item = std::make_unique<SongLyric>(GetId(), dispatcher),
  };
}

}  // namespace interface
