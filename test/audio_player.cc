#include <gmock/gmock-matchers.h>  // for StrEq, EXPECT_THAT
#include <gmock/gmock.h>
#include <gtest/gtest-message.h>    // for Message
#include <gtest/gtest-test-part.h>  // for TestPartResult

#include <chrono>
#include <memory>
#include <thread>

#include "audio/player.h"
#include "mock/decoder_mock.h"
#include "mock/interface_notifier_mock.h"
#include "mock/playback_mock.h"
#include "model/application_error.h"
#include "sync_testing.h"

namespace {

using ::testing::_;
using ::testing::Eq;
using ::testing::Field;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Return;

using testing::TestSyncer;

/**
 * @brief Tests with Player class
 */
class PlayerTest : public ::testing::Test {
  // using-declarations
  using Player = std::shared_ptr<audio::Player>;
  using NotifierMock = std::shared_ptr<InterfaceNotifierMock>;

 protected:
  void SetUp() override { Init(); }

  void TearDown() override {
    audio_player.reset();
    notifier.reset();
  }

  void Init(bool asynchronous = false) {
    // Create mocks
    PlaybackMock* pb_mock = new PlaybackMock();
    DecoderMock* dc_mock = new DecoderMock();

    // Setup init expectations
    InSequence seq;

    EXPECT_CALL(*pb_mock, CreatePlaybackStream());
    EXPECT_CALL(*pb_mock, ConfigureParameters());
    EXPECT_CALL(*pb_mock, GetPeriodSize());

    // Create Player without thread
    audio_player = audio::Player::Create(pb_mock, dc_mock, asynchronous);

    // Register interface notifier to Audio Player
    notifier = std::make_shared<InterfaceNotifierMock>();
    audio_player->RegisterInterfaceNotifier(notifier);
  }

  //! Getter for Playback (necessary as inner variable is an unique_ptr)
  auto GetPlayback() -> PlaybackMock* {
    return reinterpret_cast<PlaybackMock*>(audio_player->playback_.get());
  }

  //! Getter for Decoder (necessary as inner variable is an unique_ptr)
  auto GetDecoder() -> DecoderMock* {
    return reinterpret_cast<DecoderMock*>(audio_player->decoder_.get());
  }

  //! Getter for Public API for Player media control
  auto GetAudioControl() -> std::shared_ptr<audio::AudioControl> { return audio_player; }

  //! Run audio loop (same one executed as a thread in the real-life)
  void RunAudioLoop() { audio_player->AudioHandler(); }

 protected:
  Player audio_player;    //!< Audio player responsible for playing songs
  NotifierMock notifier;  //!< API for audio player to send interface events
};

/* ********************************************************************************************** */

class PlayerTestThread : public PlayerTest {
 protected:
  void SetUp() override { Init(true); }
};

TEST_F(PlayerTestThread, CreateDummyPlayer) {
  // Dummy testing to check setup expectation, and then, exit
  audio_player->Exit();
}

/* ********************************************************************************************** */

TEST_F(PlayerTest, CreatePlayerAndStartPlaying) {
  auto player = [&](TestSyncer& syncer) {
    auto playback = GetPlayback();
    auto decoder = GetDecoder();

    // Received filepath to play
    const std::string expected_name{"The Police - Roxanne"};

    // Setup all expectations
    InSequence seq;

    EXPECT_CALL(*decoder, OpenFile(Field(&model::Song::filepath, expected_name)))
        .WillOnce(Return(error::kSuccess));

    EXPECT_CALL(*notifier, NotifySongInformation(_));

    EXPECT_CALL(*playback, Prepare()).WillOnce(Return(error::kSuccess));

    // Only interested in second argument, which is a lambda created internally by audio_player
    // itself So it is necessary to manually call it, to keep the behaviour similar to a
    // real-life situation
    EXPECT_CALL(*decoder, Decode(_, _))
        .WillOnce(Invoke([](int dummy, driver::Decoder::AudioCallback callback) {
          callback(0, 0, 0, 0);
          return error::kSuccess;
        }));

    EXPECT_CALL(*playback, AudioCallback(_, _, _));

    EXPECT_CALL(*notifier, ClearSongInformation()).WillOnce(Invoke([&] { syncer.NotifyStep(2); }));

    // Notify that expectations are set, and run audio loop
    syncer.NotifyStep(1);
    RunAudioLoop();
  };

  auto client = [&](TestSyncer& syncer) {
    auto player_ctl = GetAudioControl();
    syncer.WaitForStep(1);

    const std::string filename{"The Police - Roxanne"};

    // Ask Audio Player to play file
    player_ctl->Play(filename);

    // Wait for Player to finish playing song before client asks to exit
    syncer.WaitForStep(2);
    player_ctl->Exit();
  };

  testing::RunAsyncTest({player, client});
}

/* ********************************************************************************************** */

TEST_F(PlayerTest, StartPlayingAndPause) {
  auto player = [&](TestSyncer& syncer) {
    auto playback = GetPlayback();
    auto decoder = GetDecoder();

    // Received filepath to play
    const std::string expected_name{"The Weeknd - Blinding Lights"};

    // Setup all expectations
    EXPECT_CALL(*decoder, OpenFile(Field(&model::Song::filepath, expected_name)))
        .WillOnce(Return(error::kSuccess));
    EXPECT_CALL(*notifier, NotifySongInformation(_));

    // Prepare is called again right after Pause was called
    EXPECT_CALL(*playback, Prepare()).Times(2).WillRepeatedly(Return(error::kSuccess));

    // Only interested in second argument, which is a lambda created internally by audio_player
    // itself So it is necessary to manually call it, to keep the behaviour similar to a
    // real-life situation
    EXPECT_CALL(*decoder, Decode(_, _))
        .WillOnce(Invoke([&](int dummy, driver::Decoder::AudioCallback callback) {
          syncer.NotifyStep(2);
          callback(0, 0, 0, 0);
          return error::kSuccess;
        }));

    EXPECT_CALL(*playback, Pause());

    EXPECT_CALL(*playback, AudioCallback(_, _, _));

    EXPECT_CALL(*notifier, ClearSongInformation()).WillOnce(Invoke([&] { syncer.NotifyStep(3); }));

    // Notify that expectations are set, and run audio loop
    syncer.NotifyStep(1);
    RunAudioLoop();
  };

  auto client = [&](TestSyncer& syncer) {
    auto player_ctl = GetAudioControl();
    syncer.WaitForStep(1);
    const std::string filename{"The Weeknd - Blinding Lights"};

    // Ask Audio Player to play file and instantly pause it
    player_ctl->Play(filename);
    player_ctl->PauseOrResume();

    // Wait until Player starts decoding before client asks to resume
    syncer.WaitForStep(2);
    player_ctl->PauseOrResume();

    // Wait for Player to finish playing song before client asks to exit
    syncer.WaitForStep(3);
    player_ctl->Exit();
  };

  testing::RunAsyncTest({player, client});
}

/* ********************************************************************************************** */

TEST_F(PlayerTest, StartPlayingAndStop) {
  auto player = [&](TestSyncer& syncer) {
    auto playback = GetPlayback();
    auto decoder = GetDecoder();

    // Received filepath to play
    const std::string expected_name{"RÜFÜS - Innerbloom (What So Not Remix)"};

    // Setup all expectations
    EXPECT_CALL(*decoder, OpenFile(Field(&model::Song::filepath, expected_name)))
        .WillOnce(Return(error::kSuccess));
    EXPECT_CALL(*notifier, NotifySongInformation(_)).WillOnce(Invoke([&] {
      // Notify step here to give enough time for client to ask for stop
      syncer.NotifyStep(2);
    }));

    // Prepare is called again right after Pause was called
    EXPECT_CALL(*playback, Prepare()).WillOnce(Return(error::kSuccess));

    // Only interested in second argument, which is a lambda created internally by audio_player
    // itself So it is necessary to manually call it, to keep the behaviour similar to a
    // real-life situation
    EXPECT_CALL(*decoder, Decode(_, _))
        .WillOnce(Invoke([&](int dummy, driver::Decoder::AudioCallback callback) {
          callback(0, 0, 0, 0);
          return error::kSuccess;
        }));

    EXPECT_CALL(*playback, AudioCallback(_, _, _)).Times(0);
    EXPECT_CALL(*playback, Stop());

    EXPECT_CALL(*notifier, ClearSongInformation()).WillOnce(Invoke([&] { syncer.NotifyStep(3); }));

    // Notify that expectations are set, and run audio loop
    syncer.NotifyStep(1);
    RunAudioLoop();
  };

  auto client = [&](TestSyncer& syncer) {
    auto player_ctl = GetAudioControl();
    syncer.WaitForStep(1);
    const std::string filename{"RÜFÜS - Innerbloom (What So Not Remix)"};

    // Ask Audio Player to play file
    player_ctl->Play(filename);

    // Wait for Player to prepare for playing
    syncer.WaitForStep(2);
    player_ctl->Stop();

    // Wait for Player to finish playing song before client asks to exit
    syncer.WaitForStep(3);
    player_ctl->Exit();
  };

  testing::RunAsyncTest({player, client});
}

/* ********************************************************************************************** */

TEST_F(PlayerTest, StartPlayingAndUpdateSongState) {
  auto player = [&](TestSyncer& syncer) {
    auto playback = GetPlayback();
    auto decoder = GetDecoder();

    // Received filepath to play
    const std::string expected_name{"The White Stripes - Blue Orchid"};

    // Setup all expectations
    EXPECT_CALL(*decoder, OpenFile(Field(&model::Song::filepath, expected_name)))
        .WillOnce(Return(error::kSuccess));

    EXPECT_CALL(*notifier, NotifySongInformation(_));

    // Prepare is called again right after Pause was called
    EXPECT_CALL(*playback, Prepare()).WillOnce(Return(error::kSuccess));

    // Only interested in second argument, which is a lambda created internally by audio_player
    // itself So it is necessary to manually call it, to keep the behaviour similar to a
    // real-life situation
    EXPECT_CALL(*decoder, Decode(_, _))
        .WillOnce(Invoke([&](int dummy, driver::Decoder::AudioCallback callback) {
          callback(0, 0, 0, 1);
          return error::kSuccess;
        }));

    EXPECT_CALL(*playback, AudioCallback(_, _, _));

    // In this case, decoder will tell us that the current timestamp matches some position other
    // than zero (this value is represented in seconds). And for this, we should notify Media Player
    // to update its graphical interface
    uint32_t expected_position = 1;
    EXPECT_CALL(*notifier, NotifySongState(Field(&model::Song::CurrentInformation::position,
                                                 expected_position)));

    EXPECT_CALL(*notifier, ClearSongInformation()).WillOnce(Invoke([&] { syncer.NotifyStep(2); }));

    // Notify that expectations are set, and run audio loop
    syncer.NotifyStep(1);
    RunAudioLoop();
  };

  auto client = [&](TestSyncer& syncer) {
    auto player_ctl = GetAudioControl();
    syncer.WaitForStep(1);
    const std::string filename{"The White Stripes - Blue Orchid"};

    // Ask Audio Player to play file
    player_ctl->Play(filename);

    // Wait for Player to finish playing song before client asks to exit
    syncer.WaitForStep(2);
    player_ctl->Exit();
  };

  testing::RunAsyncTest({player, client});
}

/* ********************************************************************************************** */

TEST_F(PlayerTest, ErrorOpeningFile) {
  auto player = [&](TestSyncer& syncer) {
    auto playback = GetPlayback();
    auto decoder = GetDecoder();

    // Received filepath to play
    const std::string expected_name{"Cannons - Round and Round"};

    // Setup all expectations
    EXPECT_CALL(*decoder, OpenFile(Field(&model::Song::filepath, expected_name)))
        .WillOnce(Return(error::kFileNotSupported));

    // None of these should be called in this situation
    EXPECT_CALL(*notifier, NotifySongInformation(_)).Times(0);
    EXPECT_CALL(*playback, Prepare()).Times(0);
    EXPECT_CALL(*decoder, Decode(_, _)).Times(0);
    EXPECT_CALL(*playback, AudioCallback(_, _, _)).Times(0);

    // Only these should be called
    EXPECT_CALL(*notifier, ClearSongInformation());
    EXPECT_CALL(*notifier, NotifyError(Eq(error::kFileNotSupported))).WillOnce(Invoke([&] {
      syncer.NotifyStep(2);
    }));

    // Notify that expectations are set, and run audio loop
    syncer.NotifyStep(1);
    RunAudioLoop();
  };

  auto client = [&](TestSyncer& syncer) {
    auto player_ctl = GetAudioControl();
    syncer.WaitForStep(1);
    const std::string filename{"Cannons - Round and Round"};

    // Ask Audio Player to play file
    player_ctl->Play(filename);

    // Wait for Player to notify error before client asks to exit
    syncer.WaitForStep(2);
    player_ctl->Exit();
  };

  testing::RunAsyncTest({player, client});
}

/* ********************************************************************************************** */

TEST_F(PlayerTest, ErrorDecodingFile) {
  auto player = [&](TestSyncer& syncer) {
    auto playback = GetPlayback();
    auto decoder = GetDecoder();

    // Received filepath to play
    const std::string expected_name{"Yung Buda - Sozinho no Tougue"};

    // Setup all expectations
    EXPECT_CALL(*decoder, OpenFile(Field(&model::Song::filepath, expected_name)))
        .WillOnce(Return(error::kSuccess));

    EXPECT_CALL(*notifier, NotifySongInformation(_));
    EXPECT_CALL(*playback, Prepare()).WillOnce(Return(error::kSuccess));

    EXPECT_CALL(*decoder, Decode(_, _)).WillOnce(Return(error::kUnknownError));

    // This should not be called in this situation
    EXPECT_CALL(*playback, AudioCallback(_, _, _)).Times(0);

    // Only these should be called
    EXPECT_CALL(*notifier, ClearSongInformation());
    EXPECT_CALL(*notifier, NotifyError(Eq(error::kUnknownError))).WillOnce(Invoke([&] {
      syncer.NotifyStep(2);
    }));

    // Notify that expectations are set, and run audio loop
    syncer.NotifyStep(1);
    RunAudioLoop();
  };

  auto client = [&](TestSyncer& syncer) {
    auto player_ctl = GetAudioControl();
    syncer.WaitForStep(1);
    const std::string filename{"Yung Buda - Sozinho no Tougue"};

    // Ask Audio Player to play file
    player_ctl->Play(filename);

    // Wait for Player to notify error before client asks to exit
    syncer.WaitForStep(2);
    player_ctl->Exit();
  };

  testing::RunAsyncTest({player, client});
}

/* ********************************************************************************************** */

TEST_F(PlayerTest, ChangeVolume) {
  auto playback = GetPlayback();
  auto player_ctl = GetAudioControl();

  // As playback is just an interface, use this variable to hold volume information and setup
  // expectation for playback from player to always return the same variable
  model::Volume value;
  EXPECT_CALL(*playback, GetVolume()).WillRepeatedly(Invoke([&] { return value; }));

  // Setup expectation for default value on volume
  EXPECT_THAT(player_ctl->GetAudioVolume(), Eq(model::Volume{1.f}));

  // Setup expectation for playback and set new volume on player
  EXPECT_CALL(*playback, SetVolume(_)).WillOnce(Invoke([&](model::Volume other) {
    value = other;
    return error::kSuccess;
  }));

  player_ctl->SetAudioVolume({0.3f});

  // Get updated volume from player
  EXPECT_THAT(player_ctl->GetAudioVolume(), Eq(model::Volume{0.3f}));

  // TODO: return error::Code on player API and create a test forcing error on volume change
}

}  // namespace
