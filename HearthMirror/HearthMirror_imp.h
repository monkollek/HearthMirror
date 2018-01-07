/*
 *  HearthMirror.h
 *  HearthMirror wrapper class
 *
 *  Created by Istvan Fehervari on 26/12/2016.
 *  Copyright © 2016 com.ifehervari. All rights reserved.
 *
 */

#import <Foundation/Foundation.h>
#define EXPORT __attribute__((visibility("default")))

@interface MirrorBrawlInfo: NSObject
    @property NSNumber *_Nullable maxWins;
    @property NSNumber *_Nullable maxLosses;
    @property BOOL isSessionBased;
    @property NSNumber *_Nonnull wins;
    @property NSNumber *_Nonnull losses;
    @property NSNumber *_Nonnull gamesPlayed;
    @property NSNumber *_Nonnull winStreak;
@end

@interface MirrorDungeonInfo: NSObject
    @property NSArray<NSNumber*> *_Nonnull bossesDefeated;
    @property NSNumber *_Nonnull bossesLostTo;
    @property NSNumber *_Nonnull nextBossHealth;
    @property NSNumber *_Nonnull heroHealth;
    @property NSArray<NSNumber*> *_Nonnull dbfIds;
    @property NSArray<NSNumber*> *_Nonnull cardsAddedToDeck;
    @property NSNumber *_Nonnull heroCardClass;
    @property NSArray<NSNumber*> *_Nonnull passiveBuffs;
    @property NSNumber *_Nonnull nextBossDbfId;
    @property NSArray<NSNumber*> *_Nonnull lootA;
    @property NSArray<NSNumber*> *_Nonnull lootB;
    @property NSArray<NSNumber*> *_Nonnull lootC;
    @property NSArray<NSNumber*> *_Nonnull treasure;
    @property NSArray<NSNumber*> *_Nonnull lootHistory;
    @property NSNumber *_Nonnull playerChosenLoot;
    @property NSNumber *_Nonnull playerChosenTreasure;
    @property BOOL runActive;
@end

@interface MirrorGameServerInfo: NSObject
    @property NSString *_Nonnull address;
    @property NSString *_Nonnull auroraPassword;
    @property NSNumber *_Nonnull clientHandle;
    @property NSNumber *_Nonnull gameHandle;
    @property NSNumber *_Nonnull mission;
    @property NSNumber *_Nonnull port;
    @property BOOL resumable;
    @property BOOL spectatorMode;
    @property NSString *_Nonnull spectatorPassword;
    @property NSString *_Nonnull version;
@end

@interface MirrorPlayer: NSObject
    @property NSString *_Nonnull name;
    @property NSNumber *_Nonnull playerId;
    @property NSNumber *_Nonnull standardRank;
    @property NSNumber *_Nonnull standardLegendRank;
    @property NSNumber *_Nonnull standardStars;
    @property NSNumber *_Nonnull wildRank;
    @property NSNumber *_Nonnull wildLegendRank;
    @property NSNumber *_Nonnull wildStars;
    @property NSNumber *_Nonnull cardBackId;
@end

@interface MirrorMatchInfo: NSObject
    @property MirrorPlayer *_Nonnull localPlayer;
    @property MirrorPlayer *_Nonnull opposingPlayer;
    @property NSNumber *_Nonnull brawlSeasonId;
    @property NSNumber *_Nonnull missionId;
    @property NSNumber *_Nonnull rankedSeasonId;
    @property NSNumber *_Nonnull gameType;
    @property NSNumber *_Nonnull formatType;
    @property BOOL spectator;
@end

@interface MirrorAccountId: NSObject
    @property NSNumber *_Nonnull lo;
    @property NSNumber *_Nonnull hi;
@end

@interface MirrorHeroLevel: NSObject
    @property NSNumber *_Nonnull heroClass;
    @property NSNumber *_Nonnull level;
    @property NSNumber *_Nonnull maxLevel;
    @property NSNumber *_Nonnull xp;
    @property NSNumber *_Nonnull maxXp;
@end

@interface MirrorCard: NSObject
    @property NSString *_Nonnull cardId;
    @property NSNumber *_Nonnull count;
    @property BOOL premium;
@end

@interface MirrorDeck: NSObject
    @property NSNumber *_Nonnull id;
    @property NSString *_Nonnull name;
    @property NSString *_Nonnull hero;
    @property BOOL isWild;
    @property NSNumber *_Nonnull type;
    @property NSNumber *_Nonnull seasonId;
    @property NSNumber *_Nonnull cardBackId;
    @property NSNumber *_Nonnull heroPremium;
    @property NSArray<MirrorCard*> *_Nonnull cards;
@end

@interface MirrorRewardData: NSObject
@end

@interface MirrorArcaneDustRewardData : MirrorRewardData
    @property NSNumber *_Nonnull amount;
@end

@interface MirrorBoosterPackRewardData : MirrorRewardData
    @property NSNumber *_Nonnull boosterId;
    @property NSNumber *_Nonnull count;
@end

@interface MirrorCardRewardData : MirrorRewardData
    @property NSString *_Nonnull cardId;
    @property NSNumber *_Nonnull count;
    @property BOOL premium;
@end

@interface MirrorCardBackRewardData : MirrorRewardData
    @property NSNumber *_Nonnull cardbackId;
@end

@interface MirrorForgeTicketRewardData : MirrorRewardData
    @property NSNumber *_Nonnull quantity;
@end

@interface MirrorGoldRewardData : MirrorRewardData
    @property NSNumber *_Nonnull amount;
@end

@interface MirrorMountRewardData : MirrorRewardData
    @property NSNumber *_Nonnull mountType;
@end

@interface MirrorArenaInfo: NSObject
    @property MirrorDeck *_Nonnull deck;
    @property NSNumber *_Nonnull losses;
    @property NSNumber *_Nonnull wins;
    @property NSNumber *_Nonnull currentSlot;
    @property NSArray<MirrorRewardData*> *_Nonnull rewards;
@end

EXPORT @interface HearthMirror : NSObject

-(nonnull instancetype) initWithPID:(pid_t)pid withBlocking:(BOOL)blocking NS_SWIFT_NAME(init(pid:blocking:));

/** Returns the BattleTag as a single string in the following format: name#number. */
-(nullable NSString*) getBattleTag;

-(nonnull NSArray<MirrorCard*>*) getCardCollection;

-(nonnull NSArray<MirrorHeroLevel*>*) getHeroLevels;

-(nullable MirrorGameServerInfo*) getGameServerInfo;

-(nullable NSNumber*) getGameType;

-(nullable MirrorMatchInfo*) getMatchInfo;

-(nullable NSNumber*) getFormat;

-(BOOL) isSpectating;

-(nullable MirrorAccountId*) getAccountId;

-(nonnull NSArray<MirrorDeck*>*) getDecks;

-(nullable NSNumber*) getSelectedDeck;

-(nullable MirrorDeck*) getEditedDeck;

-(nullable MirrorArenaInfo*) getArenaDeck;

-(nonnull NSArray<MirrorCard*>*) getArenaDraftChoices;

-(nonnull NSArray<MirrorCard*>*) getPackCards;

-(nullable MirrorBrawlInfo *) getBrawlInfo;

-(nullable MirrorDungeonInfo *) getDungeonInfo;

@end
