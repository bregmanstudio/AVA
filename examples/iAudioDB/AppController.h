//
//  AppController.h
//  CAMUS
//
//  Created by Mike Jewell on 27/01/2010.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include <audioDB_API.h>


@interface AppController : NSObject {
	adb_t *db;
	NSModalSession session;
	
	NSString* dbName;
	NSString* dbFilename;
	NSString* plistFilename;
	NSString* selectedKey;
	NSString* selectedFilename;
	
	IBOutlet NSTextField *statusField;
	IBOutlet NSTableView* tracksView;
	IBOutlet id mainWindow;

	// Query Customizing
	
	IBOutlet NSButton* multipleCheckBox;
	IBOutlet NSButton* resetButton;
	IBOutlet NSTextField* queryStartVectors;
	IBOutlet NSTextField* queryStartSeconds;
	IBOutlet NSTextField* queryLengthVectors;
	IBOutlet NSTextField* queryLengthSeconds;
	IBOutlet NSTextField* queryPath;
	IBOutlet NSButton* queryButton;
	
	// Main window buttons/fields.
	
	IBOutlet NSToolbarItem* importAudioButton;
	IBOutlet NSToolbarItem* performQueryButton;
	IBOutlet NSToolbarItem* playBothButton;
	IBOutlet NSToolbarItem* playResultButton;
	IBOutlet NSToolbarItem* stopButton;
	IBOutlet NSTextField* queryKey;
	
	NSSound* queryTrack;
	NSSound* resultTrack;
	
	// Creating
	IBOutlet id createSheet;
	IBOutlet id querySheet;
	
	IBOutlet NSMatrix* extractorOptions;
	IBOutlet NSTextField* hopSizeField;
	IBOutlet NSTextField* maxTracksField;
	IBOutlet NSTextField* maxLengthField;
	
	// Extracting
	IBOutlet id importSheet;
	IBOutlet NSProgressIndicator* indicator;
	
	NSMutableArray* results;
	NSDictionary* trackMap;
	NSDictionary* dbState;
}

//  Menus
-(IBAction)newDatabase:(id)sender;
-(IBAction)openDatabase:(id)sender;

// Import
-(IBAction)importAudio:(id)sender;
// -(IBAction)cancelImport:(id)sender;

// Create
-(IBAction)cancelCreate:(id)sender;
-(IBAction)createDatabase:(id)sender;

// Query
-(IBAction)pathAction:(id)sender;
-(IBAction)cancelQuery:(id)sender;
-(IBAction)performQuery:(id)sender;
-(IBAction)selectQueryFile:(id)sender;
-(IBAction)resetLengths:(id)sender;

// Buttons
-(IBAction)playBoth:(id)sender;
-(IBAction)playResult:(id)sender;
-(IBAction)stopPlay:(id)sender;
-(IBAction)chooseQuery:(id)sender;
-(IBAction)selectedChanged:(id)sender;
-(IBAction)tableDoubleClick:(id)sender;

-(void)reset;
-(void)updateStatus;
-(BOOL)validateUserInterfaceItem:(id <NSValidatedUserInterfaceItem>)anItem;
-(void)sound:(NSSound *)sound didFinishPlaying:(BOOL)playbackSuccessful;
-(void)importFile:(NSString *)filename withExtractorConfig:(NSString *)extractorPath;
-(UInt64)getSampleRate:(NSString *)filename;
-(UInt64)getHopSizeInSamples:(NSString *)filename;
-(int)nearestPow2:(int)x;

@end
