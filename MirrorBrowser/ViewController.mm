//
//  ViewController.m
//  MirrorBrowser
//
//  Created by Istvan Fehervari on 14/06/2017.
//  Copyright © 2017 com.ifehervari. All rights reserved.
//

#import "ViewController.h"
#import "Mirror.hpp"
#include "memhelper.h"
#include "offsets.h"

#include "MonoImage.hpp"
#include "MonoObject.hpp"
#include "MonoStruct.hpp"

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

@implementation TreeElement

-(instancetype) init {
    
    self = [super init];
    if (self)
    {
        self.name = @"";
        self.type = @"";
        self.value = @"";
        self.children = [NSMutableArray new];
    }
    return self;
}

-(void) loadChildren {
    for (TreeElement* i in self.children) {
        i.children = getChildren(i.mv);
    }
}

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    self.treeElements = [NSMutableArray new];
}


- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}

- (NSInteger)outlineView:(NSOutlineView *)outlineView
  numberOfChildrenOfItem:(id)item {
    if (item) {
        TreeElement* element = (TreeElement*)item;
        return [element.children count];
    }
    
    return [self.treeElements count];
}

// Returns a Boolean value that indicates whether the a given item is expandable.
- (BOOL)outlineView:(NSOutlineView *)outlineView
   isItemExpandable:(id)item {
    
    if (item) {
        TreeElement* element = (TreeElement*)item;
        return [element.children count] > 0;
    }
    
    return NO;
}
/*
- (BOOL)outlineView:(NSOutlineView *)outlineView
   shouldExpandItem:(id)item {
    if (item) {
        TreeElement* element = (TreeElement*)item;
        [element loadChildren];
        [outlineView reloadData];
    }
    return YES;
}*/

- (id)outlineView:(NSOutlineView *)outlineView
            child:(NSInteger)index
           ofItem:(id)item {
    
    if (item) {
        TreeElement* element = (TreeElement*)item;
        return [element.children objectAtIndex:index];
    }
    
    return [self.treeElements objectAtIndex:index];
}

// Invoked by outlineView to return the data object associated with the specified item.
/*- (id)outlineView:(NSOutlineView *)outlineView
objectValueForTableColumn:(NSTableColumn *)tableColumn
           byItem:(id)item {
    return nil;
}*/

- (NSView *)outlineView:(NSOutlineView *)outlineView
     viewForTableColumn:(NSTableColumn *)tableColumn
                   item:(id)item {
    
    TreeElement* element = (TreeElement*)item;
    NSTableCellView* cellview;
    
    if ([tableColumn.identifier isEqualToString:@"ObjectColumn"]) {
        cellview = [self.tableView makeViewWithIdentifier:@"ObjectCell" owner:self];
        
        cellview.textField.stringValue = element.name;
        //[cellview.textField sizeToFit];
    } else if ([tableColumn.identifier isEqualToString:@"TypeColumn"]) {
        cellview = [self.tableView makeViewWithIdentifier:@"TypeCell" owner:self];
        
        cellview.textField.stringValue = element.type;
        //[cellview.textField sizeToFit];
    } else if ([tableColumn.identifier isEqualToString:@"ValueColumn"]) {
        cellview = [self.tableView makeViewWithIdentifier:@"ValueCell" owner:self];
        
        cellview.textField.stringValue = element.value;
        //[cellview.textField sizeToFit];
    }
    
    
    
    return cellview;
}

NSString* MonoTypeEnumToString(MonoTypeEnum type) {
    switch (type) {
        case MONO_TYPE_END: return @"End";
        case MONO_TYPE_VOID: return @"Void";
        case MONO_TYPE_BOOLEAN: return @"Boolean";
        case MONO_TYPE_CHAR: return @"Char";
        case MONO_TYPE_I1: return @"I1";
        case MONO_TYPE_U1: return @"U1";
        case MONO_TYPE_I2: return @"I2";
        case MONO_TYPE_U2: return @"U2";
        case MONO_TYPE_I4: return @"I4";
        case MONO_TYPE_U4: return @"U4";
        case MONO_TYPE_I8: return @"I8";
        case MONO_TYPE_U8: return @"U8";
        case MONO_TYPE_R4: return @"R4";
        case MONO_TYPE_R8: return @"R8";
        case MONO_TYPE_STRING: return @"String";
        case MONO_TYPE_PTR: return @"Ptr";
        case MONO_TYPE_BYREF: return @"ByRef";
        case MONO_TYPE_VALUETYPE: return @"ValueType";
        case MONO_TYPE_CLASS: return @"Class";
        case MONO_TYPE_VAR: return @"Var";
        case MONO_TYPE_ARRAY: return @"Array";
        case MONO_TYPE_GENERICINST: return @"GenericInst";
        case MONO_TYPE_TYPEDBYREF: return @"TypedByRef";
        case MONO_TYPE_I: return @"I";
        case MONO_TYPE_U: return @"U";
        case MONO_TYPE_FNPTR: return @"FnPtr";
        case MONO_TYPE_OBJECT: return @"Object";
        case MONO_TYPE_SZARRAY: return @"Szarray";
        case MONO_TYPE_MVAR: return @"Mvar";
        case MONO_TYPE_CMOD_REQD: return @"Cmod_reqd";
        case MONO_TYPE_CMOD_OPT: return @"Cmod_opt";
        case MONO_TYPE_INTERNAL: return @"Internal";
        case MONO_TYPE_MODIFIER: return @"Modifier";
        case MONO_TYPE_SENTINEL: return @"Sentinel";
        case MONO_TYPE_PINNED: return @"Pinned";
        case MONO_TYPE_ENUM: return @"Enum";
        default: return @"Unknown";
    }
    return @"Unknown";
}

NSString* MonoTypeToString(MonoType* type) {
    return MonoTypeEnumToString(type->getType());
}

NSString* MonoValueToString(MonoValue value) {
    switch (value.type) {
        case MONO_TYPE_BOOLEAN: {
            return [NSString stringWithFormat:@"%@",value.value.b ? @"true" : @"false"];
        }
        case MONO_TYPE_U1: {
            return [NSString stringWithFormat:@"%d",value.value.u8];
        }
        case MONO_TYPE_I1: {
            return [NSString stringWithFormat:@"%d",value.value.i8];
        }
        case MONO_TYPE_I2: {
            return [NSString stringWithFormat:@"%d",value.value.i16];
        }
        case MONO_TYPE_U2: {
            return [NSString stringWithFormat:@"%d",value.value.u16];
        }
        case MONO_TYPE_CHAR: {
            return [NSString stringWithFormat:@"%c",value.value.c];
        }
        case MONO_TYPE_I:
        case MONO_TYPE_I4: {
            return [NSString stringWithFormat:@"%d",value.value.i32];
        }
        case MONO_TYPE_U:
        case MONO_TYPE_U4: {
            return [NSString stringWithFormat:@"%d",value.value.u32];
        }
        case MONO_TYPE_I8: {
            return [NSString stringWithFormat:@"%lld",value.value.i64];
        }
        case MONO_TYPE_U8: {
            return [NSString stringWithFormat:@"%lld",value.value.u64];
        }
        case MONO_TYPE_R4: {
            return [NSString stringWithFormat:@"%f",value.value.f];
        }
        case MONO_TYPE_R8:{
            return [NSString stringWithFormat:@"%f",value.value.d];
        }
        case MONO_TYPE_STRING:{
            return [NSString stringWithu16string:value.str];
        }
        default: return @"";
    }
    return @"";
}

NSMutableArray* getChildren(MonoValue value) {
    NSMutableArray* result = [NSMutableArray new];
    switch (value.type) {
        case MONO_TYPE_OBJECT:
        case MONO_TYPE_GENERICINST:
        case MONO_TYPE_VAR:
        case MONO_TYPE_CLASS: {
            MonoObject* o = value.value.obj.o;
            auto fields = o->getFields();
            for (auto it = fields.begin(); it != fields.end(); it++) {
                TreeElement* child = [TreeElement new];
                auto mv = it->second;
                child.name = [NSString stringWithCString:it->first.c_str()
                                                           encoding:[NSString defaultCStringEncoding]];
                child.type = MonoTypeEnumToString(mv.type);
                child.value = MonoValueToString(mv);
                
                if (mv.type == MONO_TYPE_OBJECT ||
                    mv.type == MONO_TYPE_VALUETYPE ||
                    mv.type == MONO_TYPE_GENERICINST ||
                    mv.type == MONO_TYPE_VAR ||
                    mv.type == MONO_TYPE_CLASS) {
                    
                    // get children
                    child.children = getChildren(mv);
                    child.mv = mv;
                }
                
                [result addObject:child];
                DeleteMonoValue(mv);
            }
            
            break;
        }
        case MONO_TYPE_VALUETYPE: {
            MonoStruct* s = value.value.obj.s;
            break;
        }
        default: break;
    }
    return result;
}

- (IBAction)reload:(id)sender {
    
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
        return;
    }
    
    MonoImage* _image;
    HANDLE task;
    try {
        if (MonoImage::getMonoImage([hsapp processIdentifier],false, &task, &_image) != 0) {
            return;
        }
    }  catch (const std::exception &e) {
        NSLog(@"Error while initializing Mirror: %s", e.what());
        return;
    }
    
    [self.treeElements removeAllObjects];
    
    NSMutableArray* queue = [NSMutableArray new];
    
    const std::map<std::string,MonoClass*> classes = _image->getClasses();
    for (auto it = classes.begin(); it != classes.end(); it++) {
        TreeElement* element = [TreeElement new];
        element.name = [NSString stringWithCString:it->first.c_str()
                                          encoding:[NSString defaultCStringEncoding]];
        element.type = @"Class";
        element.value = @"";
        
        MonoClass* c = it->second;
        auto fields = c->getFields();
        for (auto it = fields.begin(); it != fields.end(); it++) {
            auto ctype = (*it)->getType();
            
            
            TreeElement* classfieldElement = [TreeElement new];
            classfieldElement.name = [NSString stringWithCString:(*it)->getName().c_str()
                                                        encoding:[NSString defaultCStringEncoding]];
            classfieldElement.type = MonoTypeToString(ctype);
            MonoValue mv = (*it)->getStaticValue();
            
            auto cct = ctype->getType();
            if (cct == MONO_TYPE_OBJECT ||
                cct == MONO_TYPE_VALUETYPE ||
                cct == MONO_TYPE_GENERICINST ||
                cct == MONO_TYPE_VAR ||
                cct == MONO_TYPE_CLASS) {
                
                // get children
                //classfieldElement.children = getChildren(mv);
                [element.children addObject:classfieldElement];
            } else if (ctype->isStatic()) {
                classfieldElement.value = MonoValueToString(mv);
                [element.children addObject:classfieldElement];
            }
            DeleteMonoValue(mv);
            
            delete ctype;
            delete *it;
        }
        
        
        [self.treeElements addObject:element];
        [queue addObject:element];
    }

    [self.tableView reloadData];
}

@end
