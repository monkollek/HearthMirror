//
//  ViewController.h
//  MirrorBrowser
//
//  Created by Istvan Fehervari on 14/06/2017.
//  Copyright © 2017 com.ifehervari. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MonoType.hpp"

@interface TreeElement: NSObject

@property NSString *name;
@property NSString *type;
@property NSString *value;
@property NSMutableArray *children;
@property hearthmirror::MonoValue mv;

-(void) loadChildren;

@end

@interface ViewController : NSViewController<NSOutlineViewDataSource, NSOutlineViewDelegate>

@property (nonatomic, weak) IBOutlet NSOutlineView *tableView;
@property NSMutableArray *treeElements;

- (IBAction)reload:(id)sender;

@end

NSMutableArray* getChildren(hearthmirror::MonoValue value);

