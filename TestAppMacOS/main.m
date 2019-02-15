//
//  main.m
//  TestAppMacOS
//
//  Created by Istvan Fehervari on 26/12/2016.
//  Copyright © 2016 com.ifehervari. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <HearthMirror/HearthMirror.h>

#pragma mark - Get BattleTag
void loadBattleTag(HearthMirror *mirror) {
    NSString* battletag = [mirror getBattleTag];
    NSLog(@"BattleTag: %@", battletag);
}

#pragma mark - Get Account id
void loadAccountId(HearthMirror *mirror) {
    MirrorAccountId *accountId = [mirror getAccountId];
    NSLog(@"AccountId: %@", accountId);
}

#pragma mark - Get collection
void loadCollection(HearthMirror *mirror) {
    MirrorCollection* collection = [mirror getCollection];
    NSLog(@"Collection : %@", collection);
}

#pragma mark - Get hero levels
void loadHeroLevels(HearthMirror *mirror) {
    NSArray<MirrorHeroLevel*>* collection = [mirror getHeroLevels];
    NSLog(@"Collection : %@", collection);
}

#pragma mark - Get Game server info
void loadGameServerInfo(HearthMirror *mirror) {
    MirrorGameServerInfo *serverInfo = [mirror getGameServerInfo];
    NSLog(@"GameServerInfo: %@", serverInfo);
}

#pragma mark - Get the game type
void loadGameType(HearthMirror *mirror) {
    // GT_UNKNOWN = 0,
    // GT_VS_AI = 1,
    // GT_VS_FRIEND = 2,
    // GT_TUTORIAL = 4,
    // GT_ARENA = 5,
    // GT_TEST = 6,
    // GT_RANKED = 7,
    // GT_CASUAL = 8,
    // GT_TAVERNBRAWL = 16,
    // GT_TB_1P_VS_AI = 17,
    // GT_TB_2P_COOP = 18,
    // GT_LAST = 19,
    NSNumber* gameType = [mirror getGameType];
    NSLog(@"GameType: %@", gameType);
}

#pragma mark - Get the format
void loadFormat(HearthMirror *mirror) {
    // FT_UNKNOWN = 0,
    // FT_WILD = 1,
    // FT_STANDARD = 2,
    NSNumber* format = [mirror getFormat];
    NSLog(@"Format: %@", format);
}

#pragma mark - Get the match info
void loadMatchInfo(HearthMirror *mirror) {
    MirrorMatchInfo *matchInfo = [mirror getMatchInfo];
    if (matchInfo == nil) {
        NSLog(@"MatchInfo is nil, are you playing ?");
    } else {
        NSLog(@"MatchInfo: %@", matchInfo);
    }
}

#pragma mark - Get the decks
void loadDecks(HearthMirror *mirror) {
    NSArray<MirrorDeck*> *decks = [mirror getDecks];
    NSLog(@"Decks: %@", decks);
}

#pragma mark - Get the selected deck
void loadSelectedDeck(HearthMirror *mirror) {
    NSNumber *deckId = [mirror getSelectedDeck];
    NSLog(@"Selected deck: %@", deckId);
}

#pragma mark - Get info for arena
void loadArenaInfo(HearthMirror *mirror) {
    MirrorArenaInfo *arenaInfo = [mirror getArenaDeck];
    NSLog(@"Arena deck: %@", arenaInfo);
}

#pragma mark - Get cards from arena drafting
void loadArenaDraftChoice(HearthMirror *mirror) {
    NSArray<MirrorCard*> *arenaChoice = [mirror getArenaDraftChoices];
    NSLog(@"Arena Choice: %@", arenaChoice);
}

#pragma mark - Get the cards from a pack opening
void loadPackCards(HearthMirror *mirror) {
    NSArray<MirrorCard*> *packCards = [mirror getPackCards];
    NSLog(@"Pack Cards: %@", packCards);
}

#pragma mark - Get the Brawl info
void loadBrawlInfo(HearthMirror *mirror) {
    MirrorBrawlInfo *brawlInfo = [mirror getBrawlInfo];
    NSLog(@"Brawl Info: %@", brawlInfo);
}

#pragma mark - Get Dungeon info
void loadDungeonInfo(HearthMirror *mirror) {
    MirrorDungeonInfo *dungeonInfo = [mirror getDungeonInfo];
    NSLog(@"Dungeon Info: %@", dungeonInfo);
}

#pragma mark - Get the edited deck
void loadEditedDeck(HearthMirror *mirror) {
    MirrorDeck *editedDeck = [mirror getEditedDeck];
    NSLog(@"Edited deck: %@", editedDeck);
}

#pragma mark - Get the template decks
void loadTemplateDecks(HearthMirror *mirror) {
    NSArray<MirrorTemplateDeck*> *decks = [mirror getTemplateDecks];
    NSLog(@"TemplateDecks: %@", decks);
}

#pragma mark - Test entry point
int main(int argc, const char * argv[]) {
    @autoreleasepool {

        // get Hearthstone app
        NSArray* apps = [[NSWorkspace sharedWorkspace] runningApplications]; // NSRunningApplication

        NSRunningApplication* hsapp = NULL;
        for (NSRunningApplication* app in apps) {
            if ([[app bundleIdentifier] isEqualToString:@"unity.Blizzard Entertainment.Hearthstone"]) {
                hsapp = app;
                break;
            }
        }

        if (hsapp == NULL) {
            printf("Hearthstone is not running!\n");
            return 1;
        }
        
        // get rights to attach
        if (acquireTaskportRight() != 0) {
            printf("acquireTaskportRight() failed!\n");
            return 2;
        }
        
        HearthMirror* mirror = [[HearthMirror alloc] initWithPID:[hsapp processIdentifier]
                                                    withBlocking:YES];
        loadBattleTag(mirror);
        //loadDungeonInfo(mirror);
        //loadAccountId(mirror);
        //loadCollection(mirror);
        //loadGameServerInfo(mirror);
        //loadGameType(mirror);
        //loadFormat(mirror);
        //loadMatchInfo(mirror);
        //loadDecks(mirror);
        //loadSelectedDeck(mirror);
        //loadArenaInfo(mirror);
        //loadArenaDraftChoice(mirror);
        //loadPackCards(mirror);
        //loadBrawlInfo(mirror);
        //loadEditedDeck(mirror);
        //loadHeroLevels(mirror);
        loadTemplateDecks(mirror);
    }
    return 0;
}
