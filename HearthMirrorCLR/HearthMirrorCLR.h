// HearthMirrorCLR.h

#pragma once

#include "../HearthMirror/Mirror.hpp"

using namespace System;
using namespace System::Collections::Generic;

namespace HearthMirrorCLR {
	namespace Enums {
		public enum class RewardType {
			ARCANE_DUST,
			BOOSTER_PACK,
			CARD,
			CARD_BACK,
			CRAFTABLE_CARD,
			FORGE_TICKET,
			GOLD,
			MOUNT,
			CLASS_CHALLENGE
		};
		public enum class BoxState
		{
			kBoxStateInvalid,
			kBoxStateStartup,
			kBoxStatePressStart,
			kBoxStateLoading,
			kBoxStateLoadingHub,
			kBoxStateHub,
			kBoxStateHubWithDrawer,
			kBoxStateOpen,
			kBoxStateClosed,
			kBoxStateError,
			kBoxStateSetRotationLoading,
			kBoxStateSetRotation,
			kBoxStateSetRotationOpen,
		};

		public enum class UI_WINDOW
		{
			NONE,
			GENERAL_STORE,
			ARENA_STORE,
			QUEST_LOG,
		};

		public enum class SceneMode
		{
			kSceneModeInvalid,
			kSceneModeStartup,
			kSceneModeLogin,
			kSceneModeHub,
			kSceneModeGameplay,
			kSceneModeCollectionManager,
			kSceneModePackopening,
			kSceneModeTournament,
			kSceneModeFriendly,
			kSceneModeFatalError,
			kSceneModeDraft,
			kSceneModeCredits,
			kSceneModeReset,
			kSceneModeAdventure,
			kSceneModeTavernBrawl,
		};
		public enum class MirrorStatus
		{			Ok,
			ProcNotFound,
			Error
		};
	}

	namespace Objects {
		public ref class BattleTag {
		public:
			String^ Name;
			int Number;

			BattleTag(hearthmirror::_BattleTag);
			BattleTag();
		};
		public ref class Card {
		public:
			String^ Id;
			int Count;
			bool Premium;

			Card(hearthmirror::_Card card);
			Card(String^ id, int count, bool premium);
			Card();
		};
		public ref class Deck {
		public:
			long Id;
			String^ Name;
			String^ Hero;
			bool IsWild;
			int Type;
			int SeasonId;
			int CardBackId;
			int HeroPremium;
			List<Card^>^ Cards;

			Deck(hearthmirror::_Deck deck);
			Deck();
		};

		public ref class GameServerInfo {
		public:
			String^ Address;
			String^ AuroraPassword;
			long ClientHandle;
			int GameHandle;
			int Mission;
			int Port;
			bool Resumable;
			bool SpectatorMode;
			String^ SpectatorPassword;
			String^ Version;

			GameServerInfo(hearthmirror::_InternalGameServerInfo gameServerInfo);
			GameServerInfo();
		};
		public ref class MatchInfo {
		public:
			ref class Player {
			public:
				String^ Name;
				int Id;
				int StandardRank;
				int StandardLegendRank;
				int StandardStars;
				int WildRank;
				int WildLegendRank;
				int WildStars;
				int CardBackId;

				Player(hearthmirror::_InternalPlayer player);
				Player();
			};

			Player^ LocalPlayer;
			Player^ OpposingPlayer;
			int BrawlSeasonId;
			int MissionId;
			int RankedSeasonId;

			MatchInfo(hearthmirror::_InternalMatchInfo matchInfo);
			MatchInfo();
		};

		public ref class RewardData {
		public:
			Enums::RewardType Type;

			RewardData(hearthmirror::_RewardData rewardData);
			RewardData();
		};
		public ref class ArcaneDustRewardData : public HearthMirrorCLR::Objects::RewardData {
		public:
			int Amount;

			ArcaneDustRewardData(hearthmirror::_ArcaneDustRewardData arcaneDustRewardData);
			ArcaneDustRewardData();
		};
		public ref class BoosterPackRewardData : public HearthMirrorCLR::Objects::RewardData {
		public:
			int Id;
			int Count;

			BoosterPackRewardData(hearthmirror::_BoosterPackRewardData boosterPackRewardData);
			BoosterPackRewardData();
		};
		public ref class CardRewardData : public HearthMirrorCLR::Objects::RewardData {
		public:
			String^ Id;
			int Count;
			bool Premium;

			CardRewardData(hearthmirror::_CardRewardData cardRewardData);
			CardRewardData();
		};
		public ref class CardBackRewardData : public HearthMirrorCLR::Objects::RewardData {
		public:
			int Id;

			CardBackRewardData(hearthmirror::_CardBackRewardData cardBackRewardData);
			CardBackRewardData();
		};
		public ref class ForgeTicketRewardData : public HearthMirrorCLR::Objects::RewardData {
		public:
			int Quantity;

			ForgeTicketRewardData(hearthmirror::_ForgeTicketRewardData forgeTicketRewardData);
			ForgeTicketRewardData();
		};
		public ref class GoldRewardData : public HearthMirrorCLR::Objects::RewardData {
		public:
			int Amount;

			GoldRewardData(hearthmirror::_GoldRewardData goldRewardData);
			GoldRewardData(int amount);
			GoldRewardData();
		};
		public ref class MountRewardData : public HearthMirrorCLR::Objects::RewardData {
		public:
			int MountType;

			MountRewardData(hearthmirror::_MountRewardData mountRewardData);
			MountRewardData();
		};

		public ref class ArenaInfo {
		public:
			Deck^ Deck;
			int Losses;
			int Wins;
			int CurrentSlot;
			List<RewardData^>^ Rewards;

			ArenaInfo(hearthmirror::_ArenaInfo arenaInfo);
			ArenaInfo();
		};
		public ref class AccountId {
		public:
			System::UInt32 Hi;
			System::UInt32 Lo;

			AccountId(hearthmirror::_AccountId accountId);
			AccountId();
		};

		public ref class BrawlInfo {
		public:
			int MaxWins;
			int MaxLosses;
			bool IsSessionBased;
			int Wins;
			int Losses;
			int GamesPlayed;
			int WinStreak;

			BrawlInfo(hearthmirror::_BrawlInfo brawlInfo);
			BrawlInfo();
		};
		/*public ref class SeasonEndInfo {
		public:
			int BonusStars;
			int BoostedRank;
			int ChestRank;
			bool IsWild;
			int LegendRank;
			int Rank;
			int SeasonId;
			List<RewardData^>^ RankedRewards;

			SeasonEndInfo(_SeasonEndInfo seasonEndInfo);
		};*/
		public ref class SetFilterItem {
		public:
			bool IsAllStandard;
			bool IsWild;

			SetFilterItem(hearthmirror::_SetFilterItem setFilterItem);
			SetFilterItem();
		};

	}

	public ref class Reflection {
	public:
		/// <summary>
		/// Returns the battletag of the current user.
		/// </summary>
		static HearthMirrorCLR::Objects::BattleTag^ GetBattleTag();

		/// <summary>
		/// Returns the collection of the user.
		/// </summary>
		static List<HearthMirrorCLR::Objects::Card^>^ GetCollection();

		/// <summary>
		/// Returns the information about server.
		/// </summary>
		static HearthMirrorCLR::Objects::GameServerInfo^ GetServerInfo();

		/// <summary>
		/// Returns the game type.
		/// </summary>
		static int GetGameType();

		/// <summary>
		/// Returns the match information.
		/// </summary>
		static HearthMirrorCLR::Objects::MatchInfo^ GetMatchInfo();

		/// <summary>
		/// Returns the game format.
		/// </summary>
		static int GetFormat();

		/// <summary>
		/// Check if spectating.
		/// </summary>
		static bool IsSpectating();

		/// <summary>
		/// Returns the account id.
		/// </summary>
		static HearthMirrorCLR::Objects::AccountId^ GetAccountId();

		/// <summary>
		/// Returns the decks.
		/// </summary>
		static List<HearthMirrorCLR::Objects::Deck^>^ GetDecks();

		/// <summary>
		/// Returns the selected deck.
		/// </summary>
		static long GetSelectedDeckInMenu();

		/// <summary>
		/// Returns an arena deck.
		/// </summary>
		static HearthMirrorCLR::Objects::ArenaInfo^ GetArenaDeck();

		/// <summary>
		/// Returns the choice for the arena draft.
		/// </summary>
		static List<HearthMirrorCLR::Objects::Card^>^ GetArenaDraftChoices();

		/// <summary>
		/// Returns cards from an opening pack.
		/// </summary>
		static List<HearthMirrorCLR::Objects::Card^>^ GetPackCards();

		/// <summary>
		/// Returns the informations about brawl.
		/// </summary>
		static HearthMirrorCLR::Objects::BrawlInfo^ GetBrawlInfo();

		/// <summary>
		/// Returns the edited deck.
		/// </summary>
		static HearthMirrorCLR::Objects::Deck^ GetEditedDeck();

		/// <summary>
		/// Returns true if friends list is visible.
		/// </summary>
		static bool IsFriendsListVisible();

		/// <summary>
		/// Returns true if game menu is visible,
		/// </summary>
		static bool IsGameMenuVisible();

		/// <summary>
		/// Returns true if options menu is visible.
		/// </summary>
		static bool IsOptionsMenuVisible();

		/// <summary>
		/// Returns true if game is at the mulligan phase.
		/// </summary>
		static bool IsMulligan();

		/// <summary>
		/// Returns the number of mulliganed cards.
		/// </summary>
		static int GetNumMulliganCards();

		static bool IsChoosingCard();

		static int GetNumChoiceCards();

		static bool IsPlayerEmotesVisible();

		static bool IsEnemyEmotesVisible();

		static bool IsInBattlecryEffect();

		static bool IsDragging();

		static bool IsTargetingHeroPower();

		static int GetBattlecrySourceCardZonePosition();

		static bool IsHoldingCard();

		static bool IsTargetReticleActive();

		static bool IsEnemyTargeting();

		static bool IsGameOver();

		static bool IsInMainMenu();

		static HearthMirrorCLR::Enums::UI_WINDOW GetShownUiWindowId();

		static HearthMirrorCLR::Enums::SceneMode GetCurrentSceneMode();

		static bool IsPlayerHandZoneUpdatingLayout();

		static bool IsPlayerPlayZoneUpdatingLayout();

		static int GetNumCardsPlayerHand();

		static int GetNumCardsPlayerBoard();

		static int GetNavigationHistorySize();

		static int GetCurrentManaFilter();

		static HearthMirrorCLR::Objects::SetFilterItem^ GetCurrentSetFilter();

		static int GetLastOpenedBoosterId();
	private:
		static hearthmirror::Mirror* _mirror;
		static hearthmirror::Mirror* getMirror();
	};

	public ref class Status {
	public:
		HearthMirrorCLR::Enums::MirrorStatus MirrorStatus;
		System::Exception^ Exception;
		
		static Status^ GetStatus();
	private:
		Status(HearthMirrorCLR::Enums::MirrorStatus status);
		Status(System::Exception^ ex);
	};
}
