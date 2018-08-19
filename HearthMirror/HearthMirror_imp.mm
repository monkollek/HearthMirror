/*
 *  HearthMirror.mm
 *  HearthMirror
 *
 *  Created by Istvan Fehervari on 26/12/2016.
 *  Copyright © 2016 com.ifehervari. All rights reserved.
 *
 */

#import "HearthMirror_imp.h"
#import "Mirror.hpp"

#pragma mark - NSString extension
@interface NSString (cppstring_additions)
+(NSString*) stringWithu16string:(const std::u16string&)string;
@end

@implementation NSString (cppstring_additions)

+(NSString*) stringWithu16string:(const std::u16string&)ws
{
    if (ws.size() == 0) return nil;
    
    char* data = (char*)ws.data();
    
    NSString* result = [[NSString alloc] initWithBytes:data length:ws.size()*2 encoding:NSUTF16LittleEndianStringEncoding];
    return result;
}

@end

using namespace hearthmirror;

#pragma mark - HearthMirror implementation
@implementation HearthMirror {
    Mirror* _mirror;
}

-(instancetype) initWithPID:(pid_t)pid withBlocking:(BOOL)blocking {
    
    self = [super init];
    if (self)
    {
        try {
            _mirror = new Mirror(pid,blocking);
        }  catch (const std::exception &e) {
            NSLog(@"Error while initializing Mirror: %s", e.what());
            return nil;
        }
    }
    return self;
}

-(nullable NSString*) getBattleTag {
    if (_mirror == NULL) return nil;

    try {
        BattleTag tag = _mirror->getBattleTag();
        NSString* battlename = [NSString stringWithu16string:tag.name];
        return [NSString stringWithFormat:@"%@#%d",battlename,tag.number];
    } catch (const std::exception &e) {
        NSLog(@"Error while getting Battle Tag: %s", e.what());
        return nil;
    }
}
    
-(nonnull MirrorCollection*) getCollection {
    MirrorCollection* result = [MirrorCollection new];
    if (_mirror == NULL) return result;
    
    try {
        auto collection = _mirror->getCollection();
        
        NSMutableArray<MirrorCard*>* collectioncards = [NSMutableArray array];
        for (int i = 0; i < collection.cards.size(); i++) {
            Card card = collection.cards[i];
            
            MirrorCard *mirrorCard = [self buildCard:card];
            [collectioncards addObject:mirrorCard];
        }
        result.cards = collectioncards;
        
        NSMutableArray<NSNumber*>  *cardbacksMArray = [NSMutableArray array];
        for (int i = 0; i < collection.cardbacks.size(); i++) {
            auto cardback = collection.cardbacks[i];
            
            [cardbacksMArray addObject:[NSNumber numberWithInteger:cardback]];
        }
        result.cardbacks = cardbacksMArray;
        result.favoriteCardback = [NSNumber numberWithInteger:collection.favoriteCardBack];
        
        result.dust = [NSNumber numberWithInteger:collection.dust];
        result.gold = [NSNumber numberWithInteger:collection.gold];
        
        NSMutableDictionary* favoriteHeroes = [NSMutableDictionary new];
        
        for (auto it = collection.favoriteHeroes.begin(); it != collection.favoriteHeroes.end(); it++) {
            MirrorCard *mirrorCard = [self buildCard:it->second];
            [favoriteHeroes setObject:mirrorCard forKey:[NSNumber numberWithInt:it->first]];
        }
        
        result.favoriteHeroes = favoriteHeroes;
        
        return result;
    } catch (const std::exception &e) {
        NSLog(@"Error while reading cardbacks: %s", e.what());
        return result;
    }
}

-(nonnull NSArray<MirrorHeroLevel*>*) getHeroLevels {
    if (_mirror == NULL) return [NSArray array];
    
    try {
        NSMutableArray<MirrorHeroLevel*>  *result = [NSMutableArray array];
        std::vector<HeroLevel> herolevels = _mirror->getHeroLevels();
        for (int i = 0; i < herolevels.size(); i++) {
            HeroLevel hlevel = herolevels[i];
            
            MirrorHeroLevel *mirrorHeroLevel = [self buildHeroLevel:hlevel];
            [result addObject:mirrorHeroLevel];
        }
        
        return [NSArray arrayWithArray:result];
    } catch (const std::exception &e) {
        NSLog(@"Error while reading hero levels: %s", e.what());
        return [NSArray array];
    }
}

-(nullable MirrorGameServerInfo*) getGameServerInfo {
    if (_mirror == NULL) return nil;

    try {
        MirrorGameServerInfo *result = [MirrorGameServerInfo new];
        InternalGameServerInfo _serverInfo = _mirror->getGameServerInfo();
        result.address = [NSString stringWithu16string:_serverInfo.address];
        result.auroraPassword = [NSString stringWithu16string:_serverInfo.auroraPassword];
        result.clientHandle = @(_serverInfo.clientHandle);
        result.gameHandle = @(_serverInfo.gameHandle);
        result.mission = @(_serverInfo.mission);
        result.port = @(_serverInfo.port);
        result.resumable = _serverInfo.resumable;
        result.spectatorMode = _serverInfo.spectatorMode;
        result.spectatorPassword = [NSString stringWithu16string:_serverInfo.spectatorPassword];
        result.version = [NSString stringWithu16string:_serverInfo.version];

        return result;
    } catch (const std::exception &e) {
        NSLog(@"Error while getting game server info: %s", e.what());
        return nil;
    }
}

-(nullable NSNumber *) getGameType {
    if (_mirror == NULL) return nil;

    try {
        return @(_mirror->getGameType());
    } catch (const std::exception &e) {
        NSLog(@"Error while getting game type: %s", e.what());
        return nil;
    }
}

-(nullable NSNumber *) getFormat {
    if (_mirror == NULL) return nil;

    try {
        return @(_mirror->getFormat());
    } catch (const std::exception &e) {
        NSLog(@"Error while reading game format: %s", e.what());
        return nil;
    }
}

-(nullable MirrorMatchInfo *) getMatchInfo {
    if (_mirror == NULL) return nil;

    try {
        MirrorMatchInfo *result = [MirrorMatchInfo new];

        InternalMatchInfo _matchInfo = _mirror->getMatchInfo();

        MirrorPlayer *localPlayer = [self getPlayerFromPlayer:_matchInfo.localPlayer];
        if (localPlayer == nil) return nil;
        result.localPlayer = localPlayer;

        MirrorPlayer *opposingPlayer = [self getPlayerFromPlayer:_matchInfo.opposingPlayer];
        if (opposingPlayer == nil) return nil;
        result.opposingPlayer = opposingPlayer;

        result.brawlSeasonId = @(_matchInfo.brawlSeasonId);
        result.missionId = @(_matchInfo.missionId);
        result.rankedSeasonId = @(_matchInfo.rankedSeasonId);
        result.gameType = @(_matchInfo.gameType);
        result.formatType = @(_matchInfo.formatType);
        result.spectator = _matchInfo.spectator;
        
        return result;
    } catch (const std::exception &e) {
        NSLog(@"Error while readin match info: %s", e.what());
        return nil;
    }
}

-(nullable MirrorPlayer *) getPlayerFromPlayer:(InternalPlayer) player {
    if (player.name.empty()) return nil;

    MirrorPlayer *localPlayer = [MirrorPlayer new];
    localPlayer.name = [NSString stringWithu16string:player.name];
    if (localPlayer.name == nil) return nil;
    localPlayer.playerId = @(player.id);
    localPlayer.standardRank = @(player.standardRank);
    localPlayer.standardLegendRank = @(player.standardLegendRank);
    localPlayer.standardStars = @(player.standardStars);
    localPlayer.wildRank = @(player.wildRank);
    localPlayer.wildLegendRank = @(player.wildLegendRank);
    localPlayer.wildStars = @(player.wildStars);
    localPlayer.cardBackId = @(player.cardBackId);

    return localPlayer;
}

-(nullable MirrorAccountId *) getAccountId {
    if (_mirror == NULL) return nil;

    try {
        MirrorAccountId *result = [MirrorAccountId new];
        AccountId _account = _mirror->getAccountId();
        result.lo = @(_account.lo);
        result.hi = @(_account.hi);

        return result;
    } catch (const std::exception &e) {
        NSLog(@"Error while reading account Id: %s", e.what());
        return nil;
    }
}

-(MirrorDeck *)buildDeck:(Deck)deck {
    MirrorDeck *mirrorDeck = [MirrorDeck new];
    mirrorDeck.id = @(deck.id);
    mirrorDeck.name = [NSString stringWithu16string:deck.name];
    mirrorDeck.hero = [NSString stringWithu16string:deck.hero];
    mirrorDeck.isWild = deck.isWild;
    mirrorDeck.type = @(deck.type);
    mirrorDeck.seasonId = @(deck.seasonId);
    mirrorDeck.cardBackId = @(deck.cardBackId);
    mirrorDeck.heroPremium = @(deck.heroPremium);

    NSMutableArray *cards = [NSMutableArray array];
    for (int c = 0; c < deck.cards.size(); c++) {
        Card card = deck.cards[c];
        MirrorCard *mirrorCard = [self buildCard:card];
        [cards addObject:mirrorCard];
    }
    mirrorDeck.cards = [NSArray arrayWithArray:cards];

    return mirrorDeck;
}

-(MirrorCard *)buildCard:(Card)card {
    MirrorCard *mirrorCard = [MirrorCard new];
    mirrorCard.cardId = [NSString stringWithu16string:card.id];
    mirrorCard.count = @(card.count);
    mirrorCard.premium = card.premium;
    return mirrorCard;
}

-(MirrorHeroLevel *)buildHeroLevel:(HeroLevel)herolevel {
    MirrorHeroLevel *mirrorHeroLevel = [MirrorHeroLevel new];
    mirrorHeroLevel.heroClass = @(herolevel.heroClass);
    mirrorHeroLevel.level = @(herolevel.level);
    mirrorHeroLevel.maxLevel = @(herolevel.maxLevel);
    mirrorHeroLevel.xp = @(herolevel.xp);
    mirrorHeroLevel.maxXp = @(herolevel.maxXp);
    
    return mirrorHeroLevel;
}

-(nonnull NSArray<MirrorDeck*>*) getDecks {
    if (_mirror == NULL) return [NSArray array];

    try {
        NSMutableArray<MirrorDeck*> *result = [NSMutableArray array];
        std::vector<Deck> decks = _mirror->getDecks();
        for (int i = 0; i < decks.size(); i++) {
            Deck deck = decks[i];
            MirrorDeck *mirrorDeck = [self buildDeck:deck];

            [result addObject:mirrorDeck];
        }

        return [NSArray arrayWithArray:result];
    } catch (const std::exception &e) {
        NSLog(@"Error while reading decks: %s", e.what());
        return [NSArray array];
    }
}

-(nullable MirrorDeck*) getEditedDeck {
    if (_mirror == NULL) return nil;

    try {
        Deck deck = _mirror->getEditedDeck();
        return [self buildDeck:deck];
    } catch (const std::exception &e) {
        NSLog(@"Error while reading the edited deck: %s", e.what());
        return nil;
    }
}

-(BOOL) isSpectating {
    if (_mirror == NULL) return NO;

    try {
        return _mirror->isSpectating();
    } catch (const std::exception &e) {
        NSLog(@"Error while reading spectate info: %s", e.what());
        return NO;
    }
}

-(nullable NSNumber*) getSelectedDeck {
    if (_mirror == NULL) return nil;

    try {
        long deckId = _mirror->getSelectedDeckInMenu();
        if (deckId == 0) return nil;

        return @(deckId);
    } catch (const std::exception &e) {
        NSLog(@"Error while reading selected deck: %s", e.what());
        return nil;
    }
}

-(nullable MirrorArenaInfo*) getArenaDeck {
    if (_mirror == NULL) return nil;

    try {
        ArenaInfo info = _mirror->getArenaDeck();
        if (info.deck.cards.size() == 0) return nil;

        MirrorArenaInfo *arenaInfo = [MirrorArenaInfo new];
        arenaInfo.losses = @(info.losses);
        arenaInfo.wins = @(info.wins);
        arenaInfo.currentSlot = @(info.currentSlot);
        arenaInfo.deck = [self buildDeck:info.deck];
        NSMutableArray<MirrorRewardData*> *rewards = [NSMutableArray array];
        for (int i = 0; i < info.rewards.size(); i++) {
            RewardData *data = info.rewards[i];

            switch (data->type) {
                case ARCANE_DUST: {
                    ArcaneDustRewardData *_data = static_cast<ArcaneDustRewardData*>(data);
                    MirrorArcaneDustRewardData *reward = [MirrorArcaneDustRewardData new];
                    reward.amount = @(_data->amount);
                    [rewards addObject:reward];
                    break;
                }
                case BOOSTER_PACK: {
                    BoosterPackRewardData *_data = static_cast<BoosterPackRewardData*>(data);
                    MirrorBoosterPackRewardData *reward = [MirrorBoosterPackRewardData new];
                    reward.boosterId = @(_data->id);
                    reward.count = @(_data->count);
                    [rewards addObject:reward];
                    break;
                }
                case CARD: {
                    CardRewardData *_data = static_cast<CardRewardData*>(data);
                    MirrorCardRewardData *reward = [MirrorCardRewardData new];
                    reward.cardId = [NSString stringWithu16string:_data->id];
                    reward.count = @(_data->count);
                    [rewards addObject:reward];
                    break;
                }
                case CARD_BACK: {
                    CardBackRewardData *_data = static_cast<CardBackRewardData*>(data);
                    MirrorCardBackRewardData *reward = [MirrorCardBackRewardData new];
                    reward.cardbackId = @(_data->id);
                    [rewards addObject:reward];
                    break;
                }
                case FORGE_TICKET: {
                    ForgeTicketRewardData *_data = static_cast<ForgeTicketRewardData*>(data);
                    MirrorForgeTicketRewardData *reward = [MirrorForgeTicketRewardData new];
                    reward.quantity = @(_data->quantity);
                    [rewards addObject:reward];
                    break;
                }
                case GOLD: {
                    GoldRewardData *_data = static_cast<GoldRewardData*>(data);
                    MirrorGoldRewardData *reward = [MirrorGoldRewardData new];
                    reward.amount = @(_data->amount);
                    [rewards addObject:reward];
                    break;
                }
                case MOUNT: {
                    MountRewardData *_data = static_cast<MountRewardData*>(data);
                    MirrorMountRewardData *reward = [MirrorMountRewardData new];
                    reward.mountType = @(_data->mountType);
                    [rewards addObject:reward];
                    break;
                }
                case CLASS_CHALLENGE: break;
                case CRAFTABLE_CARD: break;
            }
        }
        arenaInfo.rewards = [NSArray arrayWithArray:rewards];
        
        return arenaInfo;
    } catch (const std::exception &e) {
        NSLog(@"Error while reading arena info: %s", e.what());
        return nil;
    }
}

-(nonnull NSArray<MirrorCard*>*) getArenaDraftChoices {
    if (_mirror == NULL) return [NSArray array];

    try {
        std::vector<Card> choices = _mirror->getArenaDraftChoices();
        if (choices.size() != 3) return [NSArray array];

        NSMutableArray<MirrorCard*> *cards = [NSMutableArray array];
        for (unsigned int i = 0; i < choices.size(); i++) {
            MirrorCard *mirrorCard = [self buildCard:choices[i]];
            [cards addObject:mirrorCard];
        }

        return [NSArray arrayWithArray:cards];
    } catch (const std::exception &e) {
        NSLog(@"Error while reading arena draft choices: %s", e.what());
        return [NSArray array];
    }
}

-(nonnull NSArray<MirrorCard*>*) getPackCards {
    if (_mirror == NULL) return [NSArray array];

    try {
        std::vector<Card> cards = _mirror->getPackCards();
        if (cards.size() != 5) return [NSArray array];

        NSMutableArray<MirrorCard*> *result = [NSMutableArray array];
        for (unsigned int i = 0; i < cards.size(); i++) {
            MirrorCard *mirrorCard = [self buildCard:cards[i]];
            [result addObject:mirrorCard];
        }

        return [NSArray arrayWithArray:result];
    } catch (const std::exception &e) {
        NSLog(@"Error while reading pack cards: %s", e.what());
        return [NSArray array];
    }
}

-(nullable MirrorBrawlInfo *) getBrawlInfo {
    if (_mirror == NULL) return nil;

    try {
        BrawlInfo info = _mirror->getBrawlInfo();
        MirrorBrawlInfo *result = [MirrorBrawlInfo new];
        result.maxWins = info.maxWins == -1 ? nil : @(info.maxWins);
        result.maxLosses = info.maxLosses == -1 ? nil : @(info.maxLosses);
        result.isSessionBased = info.isSessionBased;
        result.wins = @(info.wins);
        result.losses = @(info.losses);
        result.gamesPlayed = @(info.gamesPlayed);
        result.winStreak = @(info.winStreak);
        return result;
    } catch (const std::exception &e) {
        NSLog(@"Error while reading braw info: %s", e.what());
    }

    return nil;
}

NSArray* arrayFromIntVector(const std::vector<int>& v) {
    NSMutableArray *result = [NSMutableArray array];
    for (auto i = 0; i < v.size(); i++) {
        [result addObject:[NSNumber numberWithInt:v[i]]];
    }
    return result;
}

-(nullable MirrorDungeonInfo *) getDungeonInfo {
    if (_mirror == NULL) return nil;
    
    try {
        DungeonInfo info = _mirror->getDungeonInfo();
        MirrorDungeonInfo *result = [MirrorDungeonInfo new];
        
        result.bossesLostTo = [NSNumber numberWithInt:info.bossesLostTo];
        result.nextBossHealth = [NSNumber numberWithInt:info.nextBossHealth];
        result.heroHealth = [NSNumber numberWithInt:info.heroHealth];
        result.heroCardClass = [NSNumber numberWithInt:info.heroCardClass];
        result.nextBossDbfId = [NSNumber numberWithInt:info.nextBossDbfId];
        result.playerChosenLoot = [NSNumber numberWithInt:info.playerChosenLoot];
        result.playerChosenTreasure = [NSNumber numberWithInt:info.playerChosenTreasure];
        result.runActive = info.runActive;
        
        result.bossesDefeated = arrayFromIntVector(info.bossesDefeated);
        result.dbfIds = arrayFromIntVector(info.dbfIds);
        result.cardsAddedToDeck = arrayFromIntVector(info.cardsAddedToDeck);
        
        result.passiveBuffs = arrayFromIntVector(info.passiveBuffs);
        result.lootA = arrayFromIntVector(info.lootA);
        result.lootB = arrayFromIntVector(info.lootB);
        result.lootC = arrayFromIntVector(info.lootC);
        result.treasure = arrayFromIntVector(info.treasure);
        result.lootHistory = arrayFromIntVector(info.lootHistory);
        
        return result;
    } catch (const std::exception &e) {
        NSLog(@"Error while reading dungeon info: %s", e.what());
    }
    
    return nil;
}

-(void)dealloc {
    delete _mirror;
}

@end

#pragma mark - Objective-C mirror classes
@implementation MirrorGameServerInfo
- (NSString *)description {
    return [NSString stringWithFormat:@"address: %@, auroraPassword: %@, clientHandle: %@, gameHandle: %@, mission: %@, port: %@, resumable: %@, spectatorMode: %@, version: %@", self.address, self.auroraPassword, self.clientHandle, self.gameHandle, self.mission, self.port, @(self.resumable), @(self.spectatorMode), self.version];
}
@end

@implementation MirrorPlayer
- (NSString *)description {
    return [NSString stringWithFormat:@"name: %@, playerId: %@, standardRank: %@, standardLegendRank: %@, standardStars: %@, wildRank: %@, wildLegendRank: %@, wildStars: %@, cardBackId: %@", self.name, self.playerId, self.standardRank, self.standardLegendRank, self.standardStars, self.wildRank, self.wildLegendRank, self.wildStars, self.cardBackId];
}
@end

@implementation MirrorMatchInfo
- (NSString *)description {
    return [NSString stringWithFormat:@"localPlayer: [%@], opposingPlayer: [%@], brawlSeasonId: %@, missionId: %@, rankedSeasonId: %@, gameType: %@, formatType: %@, spectator: %@", self.localPlayer, self.opposingPlayer, self.brawlSeasonId, self.missionId, self.rankedSeasonId, self.gameType, self.formatType, @(self.spectator)];
}
@end

@implementation MirrorAccountId
- (NSString *)description {
    return [NSString stringWithFormat:@"hi: %@, lo: %@", self.hi, self.lo];
}
@end

@implementation MirrorCard
- (NSString *)description {
    return [NSString stringWithFormat:@"cardId: %@, count: %@, premium: %@", self.cardId, self.count, @(self.premium)];
}
@end

@implementation MirrorHeroLevel
- (NSString *)description {
    return [NSString stringWithFormat:@"class: %@, level: %@, maxLevel: %@, xp: %@, maxXp: %@", self.heroClass, self.level, self.maxLevel, self.xp, self.maxXp];
}
@end

@implementation MirrorDeck
- (NSString *)description {
    return [NSString stringWithFormat:@"id: %@, name: %@, hero: %@, isWild: %@, type: %@, seasonId: %@, cardBackId: %@, heroPremium: %@, cards: %@", self.id, self.name, self.hero, @(self.isWild), self.type, self.seasonId, self.cardBackId, self.heroPremium, self.cards];
}
@end

@implementation MirrorArenaInfo
- (NSString *)description {
    return [NSString stringWithFormat:@"deck: %@, losses: %@, wins: %@, currentSlot: %@, rewards: %@", self.deck, self.losses, self.wins, self.currentSlot, self.rewards];
}
@end

@implementation MirrorRewardData
@end

@implementation MirrorArcaneDustRewardData
- (NSString *)description {
    return [NSString stringWithFormat:@"Arcane Dust: amount: %@", self.amount];
}
@end

@implementation MirrorBoosterPackRewardData
- (NSString *)description {
    return [NSString stringWithFormat:@"Booster pack: boosterId: %@, count: %@", self.boosterId, self.count];
}
@end

@implementation MirrorCardRewardData
- (NSString *)description {
    return [NSString stringWithFormat:@"Card cardId: %@, count: %@, premium: %@", self.cardId, self.count, @(self.premium)];
}
@end

@implementation MirrorCardBackRewardData
- (NSString *)description {
    return [NSString stringWithFormat:@"CardBack cardbackId: %@", self.cardbackId];
}
@end

@implementation MirrorForgeTicketRewardData
- (NSString *)description {
    return [NSString stringWithFormat:@"ForgeTicket quantity: %@", self.quantity];
}
@end

@implementation MirrorGoldRewardData
- (NSString *)description {
    return [NSString stringWithFormat:@"Gold: amout: %@", self.amount];
}
@end

@implementation MirrorMountRewardData
- (NSString *)description {
    return [NSString stringWithFormat:@"Mount: mountType: %@", self.mountType];
}
@end

@implementation MirrorBrawlInfo
- (NSString *)description {
    return [NSString stringWithFormat:@"maxWins: %@, maxLosses: %@, isSessionBased: %@, wins: %@, losses: %@, gamesPlayed: %@, winStreak: %@", self.maxWins, self.maxLosses, @(self.isSessionBased), self.wins, self.losses, self.gamesPlayed, self.winStreak];
}
@end

@implementation MirrorDungeonInfo
- (NSString *)description {
    return [NSString stringWithFormat:@"bossesDefeated: %@, bossesLostTo: %@, nextBossHealth: %@, heroHealth: %@, dbfIds: %@, cardsAddedToDeck: %@, heroCardClass: %@, passiveBuffs: %@, nextBossDbfId: %@ lootA: %@, lootB: %@, lootC: %@, treasure: %@, lootHistory: %@, playerChosenLoot: %@, playerChosenTreasure: %@, runActive: %@", self.bossesDefeated, self.bossesLostTo, self.nextBossHealth, self.heroHealth, self.dbfIds, self.cardsAddedToDeck, self.heroCardClass, self.passiveBuffs, self.nextBossDbfId, self.lootA, self.lootB, self.lootC, self.treasure, self.lootHistory, self.playerChosenLoot, self.playerChosenTreasure, self.runActive ? @"YES" : @"NO"];
}
@end

@implementation MirrorCollection
- (NSString *)description {
    return [NSString stringWithFormat:@"Cards: %@, Cardbacks: %@, Favorite cardback: %@, Favorite Heroes: %@, Dust: %@, Gold: %@", self.cards, self.cardbacks, self.favoriteCardback, self.favoriteHeroes, self.dust, self.gold];
}
@end

