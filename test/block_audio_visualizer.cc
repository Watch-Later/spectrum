#include <gmock/gmock-matchers.h>  // for StrEq, EXPECT_THAT

#include "general/block.h"
#include "general/utils.h"  // for FilterAnsiCommands
#include "mock/event_dispatcher_mock.h"
#include "util/logger.h"
#include "view/block/audio_visualizer.h"

namespace {

using ::testing::AllOf;
using ::testing::Field;
using ::testing::StrEq;
using ::testing::VariantWith;

/**
 * @brief Tests with AudioVisualizer class
 */
class AudioVisualizerTest : public ::BlockTest {
 protected:
  static void SetUpTestSuite() { util::Logger::GetInstance().Configure(); }

  void SetUp() override {
    // Create a custom screen with fixed size
    screen = std::make_unique<ftxui::Screen>(64, 15);

    // Create mock for event dispatcher
    dispatcher = std::make_shared<EventDispatcherMock>();

    // Create AudioVisualizer block
    block = ftxui::Make<interface::AudioVisualizer>(dispatcher);
  }

  static constexpr int kNumberBars = 14;
};

/* ********************************************************************************************** */

TEST_F(AudioVisualizerTest, InitialRender) {
  auto event_bars =
      interface::CustomEvent::DrawAudioSpectrum(std::vector<double>(kNumberBars, 0.001));
  Process(event_bars);

  ftxui::Render(*screen, block->Render());

  std::string rendered = utils::FilterAnsiCommands(screen->ToString());

  std::string expected = R"(
╭ 1:visualizer 2:equalizer ─────────────────────[F1:help]───[X]╮
│                                                              │
│                                                              │
│                                                              │
│                                                              │
│                                                              │
│                                                              │
│                                                              │
│                                                              │
│                                                              │
│                                                              │
│                                                              │
│                                                              │
│   ▁▁▁ ▁▁▁ ▁▁▁ ▁▁▁ ▁▁▁ ▁▁▁ ▁▁▁ ▁▁▁ ▁▁▁ ▁▁▁ ▁▁▁ ▁▁▁ ▁▁▁ ▁▁▁    │
╰──────────────────────────────────────────────────────────────╯)";

  EXPECT_THAT(rendered, StrEq(expected));
}

/* ********************************************************************************************** */

TEST_F(AudioVisualizerTest, AnimationHorizontalMirror) {
  std::vector<double> values{0.61, 0.52, 0.41, 0.33, 0.24, 0.15, 0.06,
                             0.61, 0.52, 0.41, 0.33, 0.24, 0.15, 0.06};

  auto event_bars = interface::CustomEvent::DrawAudioSpectrum(values);
  Process(event_bars);

  ftxui::Render(*screen, block->Render());

  std::string rendered = utils::FilterAnsiCommands(screen->ToString());

  std::string expected = R"(
╭ 1:visualizer 2:equalizer ─────────────────────[F1:help]───[X]╮
│                                                              │
│                                                              │
│                                                              │
│                                                              │
│                                                              │
│                           ███ ███                            │
│                       ▇▇▇ ███ ███ ▇▇▇                        │
│                   ▃▃▃ ███ ███ ███ ███ ▃▃▃                    │
│               ▃▃▃ ███ ███ ███ ███ ███ ███ ▃▃▃                │
│           ▁▁▁ ███ ███ ███ ███ ███ ███ ███ ███ ▁▁▁            │
│           ███ ███ ███ ███ ███ ███ ███ ███ ███ ███            │
│       ███ ███ ███ ███ ███ ███ ███ ███ ███ ███ ███ ███        │
│   ▇▇▇ ███ ███ ███ ███ ███ ███ ███ ███ ███ ███ ███ ███ ▇▇▇    │
╰──────────────────────────────────────────────────────────────╯)";

  EXPECT_THAT(rendered, StrEq(expected));
}

/* ********************************************************************************************** */

TEST_F(AudioVisualizerTest, AnimationVerticalMirror) {
  std::vector<double> values{0.1, 0.2, 0.3, 0.4, 0.5, 0.4, 0.3, 0.2, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6,
                             0.1, 0.2, 0.3, 0.4, 0.5, 0.4, 0.3, 0.2, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6};

  // Expect block to send an event to terminal when 'a' is pressed
  EXPECT_CALL(*dispatcher,
              SendEvent(AllOf(
                  Field(&interface::CustomEvent::id,
                        interface::CustomEvent::Identifier::ChangeBarAnimation),
                  Field(&interface::CustomEvent::content,
                        VariantWith<int>(interface::AudioVisualizer::Animation::VerticalMirror)))));

  block->OnEvent(ftxui::Event::Character('a'));

  auto event_bars = interface::CustomEvent::DrawAudioSpectrum(values);
  Process(event_bars);

  ftxui::Render(*screen, block->Render());

  std::string rendered = utils::FilterAnsiCommands(screen->ToString());

  // Maybe filtering ansi commands is messing up with this animation =(
  std::string expected = R"(
╭ 1:visualizer 2:equalizer ─────────────────────[F1:help]───[X]╮
│                                                              │
│                                                              │
│                                                       ▅▅▅    │
│               ▄▄▄ ███ ▄▄▄                     ▄▄▄ ███ ███    │
│       ▂▂▂ ▇▇▇ ███ ███ ███ ▇▇▇ ▂▂▂     ▂▂▂ ▇▇▇ ███ ███ ███    │
│   ▅▅▅ ███ ███ ███ ███ ███ ███ ███ ▅▅▅ ███ ███ ███ ███ ███    │
│   ▃▃▃                             ▃▃▃                        │
│   ███ ▅▅▅                     ▅▅▅ ███ ▅▅▅                    │
│   ███ ███ ███ ▂▂▂     ▂▂▂ ███ ███ ███ ███ ███ ▂▂▂            │
│   ███ ███ ███ ███ ▄▄▄ ███ ███ ███ ███ ███ ███ ███ ▄▄▄        │
│   ███ ███ ███ ███ ███ ███ ███ ███ ███ ███ ███ ███ ███ ▇▇▇    │
│   ███ ███ ███ ███ ███ ███ ███ ███ ███ ███ ███ ███ ███ ███    │
│   ███ ███ ███ ███ ███ ███ ███ ███ ███ ███ ███ ███ ███ ███    │
╰──────────────────────────────────────────────────────────────╯)";

  EXPECT_THAT(rendered, StrEq(expected));
}

}  // namespace
