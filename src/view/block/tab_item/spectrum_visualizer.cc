#include "view/block/tab_item/spectrum_visualizer.h"

#include "util/logger.h"

namespace interface {

SpectrumVisualizer::SpectrumVisualizer(const std::shared_ptr<EventDispatcher>& dispatcher)
    : TabItem(dispatcher), curr_anim_{HorizontalMirror}, spectrum_data_{} {}

/* ********************************************************************************************** */

ftxui::Element SpectrumVisualizer::Render() {
  ftxui::Element bar_visualizer = ftxui::text("");

  switch (curr_anim_) {
    case Animation::HorizontalMirror:
      DrawAnimationHorizontalMirror(bar_visualizer);
      break;

    case Animation::VerticalMirror:
      DrawAnimationVerticalMirror(bar_visualizer);
      break;

    case Animation::LAST:
      ERROR("Audio visualizer current animation contains invalid value");
      curr_anim_ = HorizontalMirror;
      break;
  }

  return bar_visualizer;
}

/* ********************************************************************************************** */

bool SpectrumVisualizer::OnEvent(ftxui::Event event) {
  // Notify terminal to recalculate new size for spectrum data
  if (event == ftxui::Event::Character('a')) {
    auto dispatcher = dispatcher_.lock();
    if (!dispatcher) return false;

    spectrum_data_.clear();
    curr_anim_ = static_cast<Animation>((curr_anim_ + 1) % Animation::LAST);

    auto event = CustomEvent::ChangeBarAnimation(curr_anim_);
    dispatcher->SendEvent(event);

    return true;
  }

  return false;
}

/* ********************************************************************************************** */

bool SpectrumVisualizer::OnCustomEvent(const CustomEvent& event) {
  if (event == CustomEvent::Identifier::DrawAudioSpectrum) {
    spectrum_data_ = event.GetContent<std::vector<double>>();
    return true;
  }

  return false;
}

/* ********************************************************************************************** */

void SpectrumVisualizer::DrawAnimationHorizontalMirror(ftxui::Element& visualizer) {
  int size = spectrum_data_.size();
  if (size == 0) return;

  ftxui::Elements entries;

  // Preallocate memory
  int total_size = size * 4;
  entries.reserve(total_size);

  for (int i = (size / 2) - 1; i >= 0; i--) {
    entries.push_back(ftxui::gaugeUp(spectrum_data_[i]) | ftxui::color(ftxui::Color::SteelBlue3));
    entries.push_back(ftxui::gaugeUp(spectrum_data_[i]) | ftxui::color(ftxui::Color::SteelBlue3));
    entries.push_back(ftxui::gaugeUp(spectrum_data_[i]) | ftxui::color(ftxui::Color::SteelBlue3));
    entries.push_back(ftxui::text(" "));
  }

  for (int i = size / 2; i < size; i++) {
    entries.push_back(ftxui::gaugeUp(spectrum_data_[i]) | ftxui::color(ftxui::Color::SteelBlue3));
    entries.push_back(ftxui::gaugeUp(spectrum_data_[i]) | ftxui::color(ftxui::Color::SteelBlue3));
    entries.push_back(ftxui::gaugeUp(spectrum_data_[i]) | ftxui::color(ftxui::Color::SteelBlue3));
    entries.push_back(ftxui::text(" "));
  }

  visualizer = ftxui::hbox(std::move(entries)) | ftxui::hcenter;
}

/* ********************************************************************************************** */

void SpectrumVisualizer::DrawAnimationVerticalMirror(ftxui::Element& visualizer) {
  int size = spectrum_data_.size();
  if (size == 0) return;

  ftxui::Elements left, right;

  // Preallocate memory
  int total_size = (size / 2) * 4;
  left.reserve(total_size);
  right.reserve(total_size);

  for (int i = 0; i < size / 2; i++) {
    left.push_back(ftxui::gaugeUp(spectrum_data_[i]) | ftxui::color(ftxui::Color::SteelBlue3));
    left.push_back(ftxui::gaugeUp(spectrum_data_[i]) | ftxui::color(ftxui::Color::SteelBlue3));
    left.push_back(ftxui::gaugeUp(spectrum_data_[i]) | ftxui::color(ftxui::Color::SteelBlue3));
    left.push_back(ftxui::text(" "));
  }

  for (int i = size / 2; i < size; i++) {
    right.push_back(ftxui::gaugeDown(spectrum_data_[i]) | ftxui::color(ftxui::Color::SteelBlue3));
    right.push_back(ftxui::gaugeDown(spectrum_data_[i]) | ftxui::color(ftxui::Color::SteelBlue3));
    right.push_back(ftxui::gaugeDown(spectrum_data_[i]) | ftxui::color(ftxui::Color::SteelBlue3));
    right.push_back(ftxui::text(" "));
  }

  visualizer = ftxui::vbox(ftxui::hbox(left) | ftxui::hcenter | ftxui::yflex,
                           ftxui::hbox(right) | ftxui::hcenter | ftxui::yflex);
}

}  // namespace interface