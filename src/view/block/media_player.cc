#include "view/block/media_player.h"

#include <utility>  // for move
#include <vector>   // for vector

#include "ftxui/component/component.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/event.hpp"  // for Event

namespace interface {

constexpr int kMaxRows = 10;  //!< Maximum rows for the Component

/* ********************************************************************************************** */

MediaPlayer::MediaPlayer(const std::shared_ptr<EventDispatcher>& dispatcher)
    : Block{dispatcher, Identifier::MediaPlayer},
      btn_play_{nullptr},
      btn_stop_{nullptr},
      audio_info_{} {
  // TODO: bind methods for on_click
  btn_play_ = Button::make_button_play(nullptr);
  btn_stop_ = Button::make_button_stop(nullptr);
}

/* ********************************************************************************************** */

ftxui::Element MediaPlayer::Render() {
  // Duration
  std::string curr_time = "--:--";
  std::string total_time = "--:--";
  float position = 0;

  // Only fill these fields when exists a current song playing
  if (audio_info_.duration > 0) {
    position = (float)audio_info_.curr_state.position / (float)audio_info_.duration;
    curr_time = model::time_to_string(audio_info_.curr_state.position);
    total_time = model::time_to_string(audio_info_.duration);
  }

  ftxui::Element bar_duration = ftxui::gauge(position) | ftxui::xflex_grow |
                                ftxui::bgcolor(ftxui::Color::DarkKhaki) |
                                ftxui::color(ftxui::Color::DarkVioletBis);

  ftxui::Element bar_margin = ftxui::text("  ");

  ftxui::Element content = ftxui::vbox({ftxui::hbox({
                                            std::move(btn_play_->Render()),
                                            std::move(btn_stop_->Render()),
                                        }) | ftxui::center,
                                        ftxui::text(""),
                                        ftxui::hbox({
                                            bar_margin,
                                            std::move(bar_duration),
                                            bar_margin,
                                        }),
                                        ftxui::hbox({
                                            bar_margin,
                                            ftxui::text(curr_time) | ftxui::bold,
                                            ftxui::filler(),
                                            ftxui::text(total_time) | ftxui::bold,
                                            bar_margin,
                                        })});

  using ftxui::HEIGHT, ftxui::EQUAL;
  return ftxui::window(ftxui::text(" player "), content | ftxui::vcenter | ftxui::flex |
                                                    ftxui::size(HEIGHT, EQUAL, kMaxRows));
}

/* ********************************************************************************************** */

bool MediaPlayer::OnEvent(ftxui::Event event) {
  if (event.is_mouse()) return OnMouseEvent(event);

  return false;
}

/* ********************************************************************************************** */

bool MediaPlayer::OnCustomEvent(const CustomEvent& event) {
  // Do not return true because other blocks may use it
  if (event == CustomEvent::Identifier::ClearSongInfo) {
    audio_info_ = model::Song{};
  }

  // Do not return true because other blocks may use it
  if (event == CustomEvent::Identifier::UpdateSongInfo) {
    audio_info_ = event.GetContent<model::Song>();
  }

  if (event == CustomEvent::Identifier::UpdateSongState) {
    audio_info_.curr_state = event.GetContent<model::Song::State>();
    return true;
  }

  return false;
}

/* ********************************************************************************************** */

bool MediaPlayer::OnMouseEvent(ftxui::Event event) {
  if (btn_play_->OnEvent(event)) return true;

  if (btn_stop_->OnEvent(event)) return true;

  return false;
}

}  // namespace interface
