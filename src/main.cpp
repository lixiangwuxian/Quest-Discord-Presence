#include "beatsaber-hook/shared/utils/typedefs.h"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"
#include "beatsaber-hook/shared/utils/utils.h"
// #include "beatsaber-hook/shared/utils/logging.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/typedefs.h"
#include "beatsaber-hook/shared/config/config-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "custom-types/shared/delegate.hpp"

#include "UnityEngine/Resources.hpp"

#include "System/Action_1.hpp"


#include "GlobalNamespace/IConnectedPlayer.hpp"
#include "GlobalNamespace/MultiplayerPlayersManager.hpp"
#include "GlobalNamespace/MultiplayerSessionManager.hpp"
#include "GlobalNamespace/GameServerLobbyFlowCoordinator.hpp"
#include "GlobalNamespace/PracticeSettings.hpp"
#include "GlobalNamespace/StandardLevelDetailView.hpp"
#include "GlobalNamespace/StandardLevelScenesTransitionSetupDataSO.hpp"
#include "GlobalNamespace/BeatmapLevel.hpp"
#include "GlobalNamespace/MultiplayerLocalActivePlayerGameplayManager.hpp"
#include "GlobalNamespace/StandardLevelGameplayManager.hpp"
#include "GlobalNamespace/TutorialSongController.hpp"
#include "GlobalNamespace/MissionLevelScenesTransitionSetupDataSO.hpp"
#include "GlobalNamespace/MissionLevelGameplayManager.hpp"
#include "GlobalNamespace/PauseController.hpp"
#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "GlobalNamespace/MenuTransitionsHelper.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/ILobbyPlayersDataModel.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/LobbyPlayersDataModel.hpp"
#include "System/Collections/Generic/IReadOnlyDictionary_2.hpp"
using namespace GlobalNamespace;

#include "scotland2/shared/modloader.h"
#include "paper/shared/logger.hpp"

#include <string>
#include <optional>
#include "presencemanager.hpp"
#include <sstream> 
#include "main.hpp"

// static ModInfo modInfo;
static modloader::ModInfo modInfo{MOD_ID, VERSION, 0};
Configuration &getConfig()
{
    static Configuration config(modInfo);
    return config;
}

static PresenceManager *presenceManager = nullptr;
static LevelInfo selectedLevel;

// Converts the int representing an IBeatmapDifficulty into a string
std::string difficultyToString(BeatmapDifficulty difficulty)
{
    switch (difficulty)
    {
    case BeatmapDifficulty::Easy:
        return "Easy";
    case BeatmapDifficulty::Normal:
        return "Normal";
    case BeatmapDifficulty::Hard:
        return "Hard";
    case BeatmapDifficulty::Expert:
        return "Expert";
    case BeatmapDifficulty::ExpertPlus:
        return "Expert+";
    }
    return "Unknown";
}

// Define the current level by finding info from the IBeatmapLevel object
MAKE_HOOK_MATCH(StandardLevelDetailView_RefreshContent, &StandardLevelDetailView::RefreshContent, void, StandardLevelDetailView *self)
{
    StandardLevelDetailView_RefreshContent(self);
    BeatmapLevel *level = self->____beatmapLevel;
    if (!level)
    {
        return;
    }

    // Check if the level is an instance of BeatmapLevelSO
    selectedLevel.name = std::string(level->songName);
    std::stringstream ss;
    for (size_t i = 0; i < level->allMappers.size(); ++i) {
        // 将每个 ::StringW 转换为 std::string
        std::string mapper = static_cast<std::string>(level->allMappers[i]);

        ss << mapper;

        // 如果不是最后一个元素，添加逗号
        if (i != level->allMappers.size() - 1) {
            ss << ",";
        }
    }
    selectedLevel.levelAuthor = ss.str();  // 返回结果字符串
    selectedLevel.songAuthor = std::string(level->songAuthorName);
}

static int currentFrame = -1;

MAKE_HOOK_MATCH(MenuTransitionsHelper_StartStandardLevel, 
                static_cast<void (MenuTransitionsHelper::*)(
                    ::StringW, 
                    ByRef<::GlobalNamespace::BeatmapKey>,  // 确保使用 ByRef
                    ::GlobalNamespace::BeatmapLevel*, 
                    ::GlobalNamespace::IBeatmapLevelData*, 
                    ::GlobalNamespace::OverrideEnvironmentSettings*, 
                    ::GlobalNamespace::ColorScheme*, 
                    ::GlobalNamespace::ColorScheme*, 
                    ::GlobalNamespace::GameplayModifiers*, 
                    ::GlobalNamespace::PlayerSpecificSettings*, 
                    ::GlobalNamespace::PracticeSettings*, 
                    ::GlobalNamespace::EnvironmentsListModel*, 
                    ::StringW, 
                    bool, 
                    bool, 
                    System::Action*, 
                    System::Action_1<::Zenject::DiContainer*>*, 
                    System::Action_2<::UnityW<::GlobalNamespace::StandardLevelScenesTransitionSetupDataSO>, 
                                     ::GlobalNamespace::LevelCompletionResults*>*, 
                    System::Action_2<::UnityW<::GlobalNamespace::LevelScenesTransitionSetupDataSO>, 
                                     ::GlobalNamespace::LevelCompletionResults*>*, 
                    System::Nullable_1<::GlobalNamespace::__RecordingToolManager__SetupData>
                )>(&MenuTransitionsHelper::StartStandardLevel),
                void,
                MenuTransitionsHelper *self,
                ::StringW gameMode, 
                ByRef<::GlobalNamespace::BeatmapKey> beatmapKey,
                ::GlobalNamespace::BeatmapLevel* beatmapLevel,   
                ::GlobalNamespace::IBeatmapLevelData* beatmapLevelData, 
                ::GlobalNamespace::OverrideEnvironmentSettings* overrideEnvironmentSettings,
                ::GlobalNamespace::ColorScheme* overrideColorScheme, 
                ::GlobalNamespace::ColorScheme* beatmapOverrideColorScheme,
                ::GlobalNamespace::GameplayModifiers* gameplayModifiers, 
                ::GlobalNamespace::PlayerSpecificSettings* playerSpecificSettings,
                ::GlobalNamespace::PracticeSettings* practiceSettings, 
                ::GlobalNamespace::EnvironmentsListModel* environmentsListModel, 
                ::StringW backButtonText,
                bool useTestNoteCutSoundEffects, 
                bool startPaused, 
                ::System::Action* beforeSceneSwitchCallback, 
                ::System::Action_1<::Zenject::DiContainer*>* afterSceneSwitchCallback,
                ::System::Action_2<::UnityW<::GlobalNamespace::StandardLevelScenesTransitionSetupDataSO>, 
                ::GlobalNamespace::LevelCompletionResults*>* levelFinishedCallback,
                ::System::Action_2<::UnityW<::GlobalNamespace::LevelScenesTransitionSetupDataSO>, 
                ::GlobalNamespace::LevelCompletionResults*>* levelRestartedCallback,
                ::System::Nullable_1<::GlobalNamespace::__RecordingToolManager__SetupData> recordingToolData
)
{
    // 记录日志，标记歌曲开始
    PaperLogger.info("Song Started");

    // 初始化当前帧数为 -1
    currentFrame = -1;

    // 从传递的 difficultyBeatmap 中获取难度
    BeatmapDifficulty difficulty = beatmapKey->difficulty;
    selectedLevel.selectedDifficulty = difficultyToString(difficulty); //todo

    // 设置当前正在播放的关卡信息
    presenceManager->statusLock.lock();
    presenceManager->playingLevel.emplace(selectedLevel);
    presenceManager->isPractice = practiceSettings != nullptr; 

    if (presenceManager->isPractice)
    {
        PaperLogger.info("Practice mode is enabled!");
    }
    presenceManager->statusLock.unlock();

    // 调用原始的 StartStandardLevel 函数
    MenuTransitionsHelper_StartStandardLevel(
        self,
        gameMode,
        beatmapKey,  // 确保传递 ByRef 参数
        beatmapLevel,
        beatmapLevelData,
        overrideEnvironmentSettings,
        overrideColorScheme,
        beatmapOverrideColorScheme,
        gameplayModifiers,
        playerSpecificSettings,
        practiceSettings,
        environmentsListModel,
        backButtonText,
        useTestNoteCutSoundEffects,
        startPaused,
        beforeSceneSwitchCallback,
        afterSceneSwitchCallback,
        levelFinishedCallback,
        levelRestartedCallback,
        recordingToolData
    );
}


// Called when starting a multiplayer level
MAKE_HOOK_MATCH(MenuTransitionsHelper_StartMultiplayerLevel, static_cast<
                    void (MenuTransitionsHelper::*)
                    (
                        ::StringW,
                        ByRef<::GlobalNamespace::BeatmapKey>, // 确保类型为 ByRef
                        ::GlobalNamespace::BeatmapLevel*,
                        ::GlobalNamespace::IBeatmapLevelData*,
                        ::GlobalNamespace::ColorScheme*,
                        ::GlobalNamespace::GameplayModifiers*,
                        ::GlobalNamespace::PlayerSpecificSettings*,
                        ::GlobalNamespace::PracticeSettings*,
                        ::StringW,
                        bool,
                        ::System::Action*,
                        ::System::Action_1<::Zenject::DiContainer*>*,
                        ::System::Action_2<::UnityW<::GlobalNamespace::MultiplayerLevelScenesTransitionSetupDataSO>, ::GlobalNamespace::MultiplayerResultsData*>*,
                        ::System::Action_1<::GlobalNamespace::DisconnectedReason>*
                    )
                >(&MenuTransitionsHelper::StartMultiplayerLevel), void,
                MenuTransitionsHelper *self,
                ::StringW gameMode,
                ByRef<::GlobalNamespace::BeatmapKey> beatmapKey,  // 确保使用 ByRef
                ::GlobalNamespace::BeatmapLevel* beatmapLevel,
                ::GlobalNamespace::IBeatmapLevelData* beatmapLevelData,
                ::GlobalNamespace::ColorScheme* overrideColorScheme,
                ::GlobalNamespace::GameplayModifiers* gameplayModifiers,
                ::GlobalNamespace::PlayerSpecificSettings* playerSpecificSettings,
                ::GlobalNamespace::PracticeSettings* practiceSettings,
                ::StringW backButtonText,
                bool useTestNoteCutSoundEffects,
                ::System::Action* beforeSceneSwitchCallback,
                ::System::Action_1<::Zenject::DiContainer*>* afterSceneSwitchCallback,
                ::System::Action_2<::UnityW<::GlobalNamespace::MultiplayerLevelScenesTransitionSetupDataSO>, ::GlobalNamespace::MultiplayerResultsData*>* levelFinishedCallback,
                ::System::Action_1<::GlobalNamespace::DisconnectedReason>* didDisconnectCallback)
{

    PaperLogger.info("Multiplayer Song Started");

    // 使用 beatmapKey 和 beatmapLevelData 来确定选中的难度
    BeatmapDifficulty difficulty = beatmapKey->difficulty;
    selectedLevel.selectedDifficulty = difficultyToString(difficulty);
    
    // // 设置正在播放的关卡
    presenceManager->statusLock.lock();
    presenceManager->playingLevel.emplace(selectedLevel);
    presenceManager->statusLock.unlock();

    // 调用原始函数
    MenuTransitionsHelper_StartMultiplayerLevel(
        self,
        gameMode,
        beatmapKey,  // 使用 ByRef 参数
        beatmapLevel,
        beatmapLevelData,
        overrideColorScheme,
        gameplayModifiers,
        playerSpecificSettings,
        practiceSettings,
        backButtonText,
        useTestNoteCutSoundEffects,
        beforeSceneSwitchCallback,
        afterSceneSwitchCallback,
        levelFinishedCallback,
        didDisconnectCallback);
}

void handleLobbyPlayersDataModelDidChange(IMultiplayerSessionManager *multiplayerSessionManager, ::StringW userId)
{
    //todo
    // presenceManager->statusLock.lock();
    // presenceManager->multiplayerLobby->numberOfPlayers = multiplayerSessionManager->connectedPlayerCount + 1;
    // presenceManager->statusLock.unlock();
}

// Reset the lobby back to null when we leave back to the menu
void onLobbyDisconnect(GlobalNamespace::DisconnectedReason reason)
{
    PaperLogger.info("Left Multiplayer lobby");
    presenceManager->statusLock.lock();
    presenceManager->multiplayerLobby = std::nullopt;
    presenceManager->statusLock.unlock();
}

MAKE_HOOK_MATCH(GameServerLobbyFlowCoordinator_DidActivate, &GameServerLobbyFlowCoordinator::DidActivate, void, GameServerLobbyFlowCoordinator *self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
{
    PaperLogger.info("Joined multiplayer lobby");

    // TODO avoid FindObjectsOfTypeAll calls if possible
    // Not too much of an issue since we only do it once on multiplayer lobby start, but still not ideal

    // Used for updating current player count in the DidChange event
    LobbyPlayersDataModel *lobbyPlayersDataModel = reinterpret_cast<LobbyPlayersDataModel *>(self->_lobbyPlayersDataModel);

    // Used for getting max player count
    // Previously used for getting current player count by listening to player connections/disconnections, however this isn't reliable, and yielded negative player counts
    // todo
    // IMultiplayerSessionManager *sessionManager = lobbyPlayersDataModel->_multiplayerSessionManager;

    // int maxPlayers = sessionManager->maxPlayerCount;
    // int numActivePlayers = sessionManager->connectedPlayerCount;

    // // Set the number of players in this lobby
    // MultiplayerLobbyInfo lobbyInfo;
    // lobbyInfo.numberOfPlayers = numActivePlayers + 1;
    // lobbyInfo.maxPlayers = maxPlayers;
    // presenceManager->statusLock.lock();
    // presenceManager->multiplayerLobby.emplace(lobbyInfo);
    // presenceManager->statusLock.unlock();

    // lobbyPlayersDataModel->add_didChangeEvent(
    //     custom_types::MakeDelegate<System::Action_1<StringW>*>(
    //         std::function<void(StringW)>(std::bind(handleLobbyPlayersDataModelDidChange, sessionManager, std::placeholders::_1))
    //     )
    // );



    // Register disconnect from lobby event
    // sessionManager->add_disconnectedEvent(
    //     custom_types::MakeDelegate<System::Action_1<GlobalNamespace::DisconnectedReason> *>(
    //         std::function<void(GlobalNamespace::DisconnectedReason)>(std::bind(onLobbyDisconnect, std::placeholders::_1))
    //     )
    // ); //todo
    GameServerLobbyFlowCoordinator_DidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);
}

// Called on standard level end
MAKE_HOOK_MATCH(StandardLevelGameplayManager_OnDestroy, &StandardLevelGameplayManager::OnDestroy, void, StandardLevelGameplayManager *self)
{
    PaperLogger.info("Song Ended");
    presenceManager->statusLock.lock();
    presenceManager->playingLevel = std::nullopt; // Reset the currently playing song to None
    presenceManager->paused = false;              // If we are pasued, unpause us, since we are returning to the menu
    presenceManager->statusLock.unlock();
    StandardLevelGameplayManager_OnDestroy(self);
}

// Called on multiplayer level end
MAKE_HOOK_MATCH(MultiplayerLocalActivePlayerGameplayManager_OnDisable, &MultiplayerLocalActivePlayerGameplayManager::OnDisable, void, MultiplayerLocalActivePlayerGameplayManager *self)
{
    PaperLogger.info("Multiplayer Song Ended");
    presenceManager->statusLock.lock();
    presenceManager->playingLevel = std::nullopt; // Reset the currently playing song to None
    presenceManager->paused = false;              // If we are pasued, unpause us, since we are returning to the menu
    presenceManager->statusLock.unlock();
    MultiplayerLocalActivePlayerGameplayManager_OnDisable(self);
}

// Called on tutorial start
MAKE_HOOK_MATCH(TutorialSongController_Awake, &TutorialSongController::Awake, void, TutorialSongController *self)
{
    PaperLogger.info("Tutorial starting");
    presenceManager->statusLock.lock();
    presenceManager->playingTutorial = true;
    presenceManager->statusLock.unlock();
    TutorialSongController_Awake(self);
}

MAKE_HOOK_MATCH(TutorialSongController_OnDestroy, &TutorialSongController::OnDestroy, void, TutorialSongController *self)
{
    PaperLogger.info("Tutorial ending");
    presenceManager->statusLock.lock();
    presenceManager->playingTutorial = false;
    presenceManager->paused = false; // If we are pasued, unpause us, since we are returning to the menu
    presenceManager->statusLock.unlock();
    TutorialSongController_OnDestroy(self);
}

MAKE_HOOK_MATCH(MissionLevelScenesTransitionSetupDataSO_Init, 
                static_cast<void (MissionLevelScenesTransitionSetupDataSO::*)(
                    ::StringW, 
                    ByRef<::GlobalNamespace::BeatmapKey>, 
                    ::GlobalNamespace::BeatmapLevel*, 
                    ::ArrayW<::GlobalNamespace::MissionObjective*, ::Array<::GlobalNamespace::MissionObjective*>*>, 
                    ::GlobalNamespace::ColorScheme*, 
                    ::GlobalNamespace::GameplayModifiers*, 
                    ::GlobalNamespace::PlayerSpecificSettings*, 
                    ::GlobalNamespace::EnvironmentsListModel*, 
                    ::GlobalNamespace::BeatmapLevelsModel*, 
                    ::GlobalNamespace::AudioClipAsyncLoader*, 
                    ::BeatSaber::PerformancePresets::PerformancePreset*, 
                    ::GlobalNamespace::BeatmapDataLoader*, 
                    ::StringW
                )>(&MissionLevelScenesTransitionSetupDataSO::Init), 
                void,
                MissionLevelScenesTransitionSetupDataSO *self,
                ::StringW missionId,
                ByRef<::GlobalNamespace::BeatmapKey> beatmapKey,
                ::GlobalNamespace::BeatmapLevel* beatmapLevel,
                ::ArrayW<::GlobalNamespace::MissionObjective*, ::Array<::GlobalNamespace::MissionObjective*>*> missionObjectives,
                ::GlobalNamespace::ColorScheme* overrideColorScheme,
                ::GlobalNamespace::GameplayModifiers* gameplayModifiers,
                ::GlobalNamespace::PlayerSpecificSettings* playerSpecificSettings,
                ::GlobalNamespace::EnvironmentsListModel* environmentsListModel,
                ::GlobalNamespace::BeatmapLevelsModel* beatmapLevelsModel,
                ::GlobalNamespace::AudioClipAsyncLoader* audioClipAsyncLoader,
                ::BeatSaber::PerformancePresets::PerformancePreset* performancePreset,
                ::GlobalNamespace::BeatmapDataLoader* beatmapDataLoader,
                ::StringW backButtonText)
{
    PaperLogger.info("Campaign level starting");

    // 设置当前帧数为 -1
    currentFrame = -1;

    // 设置状态为正在播放的Campaign
    presenceManager->statusLock.lock();
    presenceManager->playingCampaign = true;
    presenceManager->statusLock.unlock();

    // 调用原始的 Init 函数，传入所有参数
    MissionLevelScenesTransitionSetupDataSO_Init(
        self, 
        missionId, 
        beatmapKey, 
        beatmapLevel,
        missionObjectives,
        overrideColorScheme,
        gameplayModifiers,
        playerSpecificSettings,
        environmentsListModel,
        beatmapLevelsModel,
        audioClipAsyncLoader,
        performancePreset,
        beatmapDataLoader,
        backButtonText
    );
}


// Called upon mission levels (campaign levels) ending.
MAKE_HOOK_MATCH(MissionLevelGameplayManager_OnDestroy, &MissionLevelGameplayManager::OnDestroy, void, MissionLevelGameplayManager *self)
{
    PaperLogger.info("Campaign level ending");
    presenceManager->statusLock.lock();
    presenceManager->playingCampaign = false;
    presenceManager->paused = false; // If we are paused, unpause us, since we are returning to the menu
    presenceManager->statusLock.unlock();
    MissionLevelGameplayManager_OnDestroy(self);
}

MAKE_HOOK_MATCH(PauseController_Pause, &PauseController::Pause, void, PauseController *self)
{
    PaperLogger.info("Game paused");
    presenceManager->statusLock.lock();
    presenceManager->paused = true;
    presenceManager->statusLock.unlock();
    PauseController_Pause(self);
}

MAKE_HOOK_MATCH(PauseController_HandlePauseMenuManagerDidPressContinueButton, &PauseController::HandlePauseMenuManagerDidPressContinueButton, void, PauseController *self)
{
    PaperLogger.info("Game resumed");
    presenceManager->statusLock.lock();
    presenceManager->paused = false;
    presenceManager->statusLock.unlock();
    PauseController_HandlePauseMenuManagerDidPressContinueButton(self);
}

// Used to update song time - this is called every frame and is in a convenient place allowing us to easily get the song time
MAKE_HOOK_MATCH(AudioTimeSyncController_Update, &AudioTimeSyncController::Update, void, AudioTimeSyncController *self)
{
    AudioTimeSyncController_Update(self);
    // Only update the time every 36 frames or so (0.5 seconds on Q1, shorter on Q2 but whatever) to avoid log spam
    constexpr int TIME_UPDATE_INTERVAL = 36;

    currentFrame++;
    if (currentFrame % TIME_UPDATE_INTERVAL != 0)
    {
        return;
    }
    currentFrame = 0;

    float time = self->get_songTime();
    float endTime = self->get_songEndTime();

    presenceManager->statusLock.lock();
    presenceManager->timeLeft = (int)(endTime - time);
    presenceManager->statusLock.unlock();
}

void saveDefaultConfig()
{
    PaperLogger.info("Creating config file . . .");
    ConfigDocument &config = getConfig().config;
    auto &alloc = config.GetAllocator();
    // If the config has already been created, don't overwrite it
    if (config.HasMember("multiplayerLevelPresence"))
    {
        PaperLogger.info("Config file already exists");
        return;
    }
    config.RemoveAllMembers();
    config.SetObject();
    // Create the sections of the config file for each type of presence
    rapidjson::Value levelPresence(rapidjson::kObjectType);
    levelPresence.AddMember("details", "Playing {mapName} ({mapDifficulty})", alloc);
    levelPresence.AddMember("state", "By: {mapAuthor} {paused?}", alloc);
    config.AddMember("standardLevelPresence", levelPresence, alloc);

    rapidjson::Value practicePresence(rapidjson::kObjectType);
    practicePresence.AddMember("details", "Practising {mapName} ({mapDifficulty})", alloc);
    practicePresence.AddMember("state", "By: {mapAuthor} {paused?}", alloc);
    config.AddMember("practicePresence", practicePresence, alloc);

    rapidjson::Value multiLevelPresence(rapidjson::kObjectType);
    multiLevelPresence.AddMember("details", "Playing multiplayer: ({numPlayers}/{maxPlayers})", alloc);
    multiLevelPresence.AddMember("state", "{mapName} - {mapDifficulty} {paused?}", alloc);
    config.AddMember("multiplayerLevelPresence", multiLevelPresence, alloc);

    rapidjson::Value missionPresence(rapidjson::kObjectType);
    missionPresence.AddMember("details", "Playing Campaign", alloc);
    missionPresence.AddMember("state", "{paused?}", alloc);
    config.AddMember("missionLevelPresence", missionPresence, alloc);

    rapidjson::Value tutorialPresence(rapidjson::kObjectType);
    tutorialPresence.AddMember("details", "Playing Tutorial", alloc);
    tutorialPresence.AddMember("state", "{paused?}", alloc);
    config.AddMember("tutorialPresence", tutorialPresence, alloc);

    rapidjson::Value multiLobbyPresence(rapidjson::kObjectType);
    multiLobbyPresence.AddMember("details", "Multiplayer - In Lobby", alloc);
    multiLobbyPresence.AddMember("state", "with ({numPlayers}/{maxPlayers}) players", alloc);
    config.AddMember("multiplayerLobbyPresence", multiLobbyPresence, alloc);

    rapidjson::Value menuPresence(rapidjson::kObjectType);
    menuPresence.AddMember("details", "In Menu", alloc);
    menuPresence.AddMember("state", "", alloc);
    config.AddMember("menuPresence", menuPresence, alloc);

    getConfig().Write();
    PaperLogger.info("Config file created");
}

MOD_EXTERN_FUNC void setup(CModInfo *info)
{
    *info = modInfo.to_c();

    // info.id = ID;
    // info.version = VERSION;
    // modInfo = info;
    // PaperLogger.info("Modloader name: %s", Modloader::getInfo().name.c_str());
    getConfig().Load();
    saveDefaultConfig(); // Create the default config file
    Paper::Logger::RegisterFileContextId(PaperLogger.tag);
    PaperLogger.info("Completed setup!");
}

MOD_EXTERN_FUNC void load()
{
    PaperLogger.debug("Installing hooks...");
    il2cpp_functions::Init();

    // Install our function hooks
    // Logger &PaperLogger = PaperLogger;
    INSTALL_HOOK(PaperLogger, StandardLevelDetailView_RefreshContent);
    INSTALL_HOOK(PaperLogger, MenuTransitionsHelper_StartStandardLevel);
    INSTALL_HOOK(PaperLogger, StandardLevelGameplayManager_OnDestroy);
    INSTALL_HOOK(PaperLogger, MissionLevelScenesTransitionSetupDataSO_Init);
    INSTALL_HOOK(PaperLogger, MissionLevelGameplayManager_OnDestroy);
    INSTALL_HOOK(PaperLogger, TutorialSongController_Awake);
    INSTALL_HOOK(PaperLogger, TutorialSongController_OnDestroy);
    INSTALL_HOOK(PaperLogger, PauseController_Pause);
    INSTALL_HOOK(PaperLogger, PauseController_HandlePauseMenuManagerDidPressContinueButton);
    INSTALL_HOOK(PaperLogger, AudioTimeSyncController_Update);
    INSTALL_HOOK(PaperLogger, MenuTransitionsHelper_StartMultiplayerLevel);
    INSTALL_HOOK(PaperLogger, GameServerLobbyFlowCoordinator_DidActivate);
    INSTALL_HOOK(PaperLogger, MultiplayerLocalActivePlayerGameplayManager_OnDisable);

    PaperLogger.debug("Installed all hooks!");
    presenceManager = new PresenceManager(PaperLogger, getConfig().config);
}