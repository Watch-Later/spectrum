#include "view/element/button.h"

namespace interface {

Button::Button(const ButtonStyle& style, Callback on_click, bool active)
    : box_{},
      active_{active},
      focused_{false},
      clicked_{false},
      pressed_{false},
      style_{style},
      on_click_{on_click} {}

/* ********************************************************************************************** */

bool Button::OnEvent(ftxui::Event event) {
  if (event.mouse().button == ftxui::Mouse::WheelDown ||
      event.mouse().button == ftxui::Mouse::WheelUp) {
    return false;
  }

  //   if (event.mouse().button != ftxui::Mouse::None && event.mouse().button != ftxui::Mouse::Left)
  //   {
  //     return false;
  //   }

  //   if (!CaptureMouse(event)) return false;

  if (box_.Contain(event.mouse().x, event.mouse().y)) {
    focused_ = true;

    if (active_ && event.mouse().button == ftxui::Mouse::Left) {
      // Mouse click hold
      if (event.mouse().motion == ftxui::Mouse::Pressed) {
        pressed_ = true;
      }

      // Mouse click
      if (event.mouse().motion == ftxui::Mouse::Released) {
        // Update internal state
        pressed_ = false;
        ToggleState();

        // Mouse click on menu entry
        if (on_click_ != nullptr) on_click_();

        return true;
      }
    }
  } else {
    // Clear some states
    focused_ = false;
    pressed_ = false;
  }

  return false;
}

/* ********************************************************************************************** */

void Button::SetState(bool clicked) { clicked_ = clicked; }

/* ********************************************************************************************** */

void Button::ToggleState() { clicked_ = !clicked_; }

/* ********************************************************************************************** */

void Button::ResetState() { clicked_ = false; }

/* ********************************************************************************************** */

void Button::SetActive() {
  if (!active_) active_ = true;
}

/* ********************************************************************************************** */

void Button::SetInactive() {
  if (active_) active_ = false;
}

/* ********************************************************************************************** */

bool Button::IsActive() const { return active_; }

/* ********************************************************************************************** */

void Button::OnClick() {
  if (on_click_ != nullptr) on_click_();
}

/* ********************************************************************************************** */

std::shared_ptr<Button> Button::make_button_play(Callback on_click) {
  class Play : public Button {
   public:
    explicit Play(const ButtonStyle& style, Callback on_click) : Button(style, on_click, true) {}

    //! Override base class method to implement custom rendering
    ftxui::Element Render() override {
      ftxui::Canvas content = !clicked_ ? DrawPlay() : DrawPause();

      auto button =
          ftxui::canvas(std::move(content)) | ftxui::hcenter | ftxui::border | ftxui::reflect(box_);

      ftxui::Decorator border_color =
          !focused_ ? ftxui::color(style_.border_normal) : ftxui::color(style_.border_focused);

      button = button | border_color;
      return button;
    }

   private:
    //! To make life easier
    using Point = std::pair<int, int>;

    //! Draw Play button
    ftxui::Canvas DrawPlay() {
      // play
      ftxui::Canvas play(12, 12);

      auto a = Point{3, 0};
      auto b = Point{9, 6};
      auto c = Point{3, 11};

      for (int i = 1; i < 6; ++i) {
        play.DrawPointLine(a.first + i, a.second + i, b.first - i, b.second - i, style_.content);
        play.DrawPointLine(b.first - i, b.second - i, c.first + i, c.second - i, style_.content);
        play.DrawPointLine(c.first + i, c.second - i, a.first + i, a.second + i, style_.content);
      }

      return play;
    }

    // Draw Pause button
    ftxui::Canvas DrawPause() {
      // pause
      ftxui::Canvas pause(12, 12);
      auto g = Point{2, 1};
      auto h = Point{2, 10};
      int space = 6;

      for (int i = 0; i < 2; ++i) {
        pause.DrawPointLine(g.first + i, g.second, h.first + i, h.second, style_.content);
        pause.DrawPointLine(g.first + i + space, g.second, h.first + i + space, h.second,
                            style_.content);
      }

      return pause;
    }
  };

  auto style = ButtonStyle{
      .content = ftxui::Color::SpringGreen2,
      .border_normal = ftxui::Color::GrayDark,
      .border_focused = ftxui::Color::SteelBlue3,
  };

  return std::make_shared<Play>(style, on_click);
}

/* ********************************************************************************************** */

std::shared_ptr<Button> Button::make_button_stop(Callback on_click) {
  class Stop : public Button {
   public:
    explicit Stop(const ButtonStyle& style, Callback on_click) : Button(style, on_click, true) {}

    //! Override base class method to implement custom rendering
    ftxui::Element Render() override {
      // stop
      ftxui::Canvas stop(12, 12);

      for (int i = 1; i < 11; ++i) {
        stop.DrawPointLine(2, i, 9, i, style_.content);
      }

      auto button_stop =
          ftxui::canvas(std::move(stop)) | ftxui::hcenter | ftxui::border | ftxui::reflect(box_);

      ftxui::Decorator border_color =
          !focused_ ? ftxui::color(style_.border_normal) : ftxui::color(style_.border_focused);

      button_stop = button_stop | border_color;
      return button_stop;
    }
  };

  auto style = ButtonStyle{
      .content = ftxui::Color::Red,
      .border_normal = ftxui::Color::GrayDark,
      .border_focused = ftxui::Color::SteelBlue3,
  };

  return std::make_shared<Stop>(style, on_click);
}

/* ********************************************************************************************** */

std::shared_ptr<Button> Button::make_button_for_window(const std::string& content,
                                                       Callback on_click,
                                                       const Delimiters& delimiters) {
  class WindowButton : public Button {
   public:
    explicit WindowButton(const ButtonStyle& style, const std::string& content, Callback on_click)
        : Button(style, on_click, true), content_{content} {}

    //! Override base class method to implement custom rendering
    ftxui::Element Render() override {
      auto left = ftxui::text(std::get<0>(style_.delimiters)) | ftxui::bold;
      auto right = ftxui::text(std::get<1>(style_.delimiters)) | ftxui::bold;
      auto content = ftxui::text(content_);

      ftxui::Decorator decorator = ftxui::nothing;
      if (focused_) decorator = ftxui::color(style_.content) | ftxui::inverted;

      return ftxui::hbox({left, std::move(content), right}) | decorator | ftxui::reflect(box_);
    }

    std::string content_;
  };

  auto style = ButtonStyle{
      .content = ftxui::Color::White, .delimiters = delimiters,
      // not using the rest...
  };

  return std::make_shared<WindowButton>(style, content, on_click);
}

/* ********************************************************************************************** */

std::shared_ptr<Button> Button::make_button(const std::string& content, Callback on_click,
                                            bool active) {
  class GenericButton : public Button {
   public:
    explicit GenericButton(const ButtonStyle& style, const std::string& content, Callback on_click,
                           bool active)
        : Button(style, on_click, active), content_{content} {}

    //! Override base class method to implement custom rendering
    ftxui::Element Render() override {
      using ftxui::HEIGHT, ftxui::WIDTH, ftxui::EQUAL;

      auto content = ftxui::text(content_);

      // default decorator
      ftxui::Decorator decorator = ftxui::center;

      decorator = decorator | ftxui::borderLight;
      if (style_.height) decorator = decorator | ftxui::size(HEIGHT, EQUAL, style_.height);
      if (style_.width) decorator = decorator | ftxui::size(WIDTH, EQUAL, style_.width);
      if (!active_) decorator = decorator | ftxui::color(ftxui::Color::GrayDark);
      if (active_ && focused_)
        decorator = decorator | ftxui::color(style_.content) | ftxui::inverted;
      // TODO: think about this
      //   if (pressed_) decorator = decorator | ftxui::bgcolor(ftxui::Color::Aquamarine1);

      return ftxui::hbox(std::move(content)) | decorator | ftxui::reflect(box_);
    }

    std::string content_;
  };

  auto style = ButtonStyle{
      .content = ftxui::Color::White,
      .border_normal = ftxui::Color::GrayDark,
      .border_focused = ftxui::Color::SteelBlue3,
      .width = 15,
  };

  return std::make_shared<GenericButton>(style, content, on_click, active);
}

}  // namespace interface
