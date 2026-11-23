#import <UIKit/UIKit.h>
#import <CoreGraphics/CoreGraphics.h>
#import <AVFoundation/AVFoundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import <CoreLocation/CoreLocation.h>
#import <PhotosUI/PhotosUI.h>
#import <UserNotifications/UserNotifications.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "../common/salam_host_common.h"

extern int salam_app_main(int argc, char **argv);

@class SalamView;

static SalamView *g_view;
static void salam_device_init(void);
static CGFloat g_kb;
static pthread_t g_thread;
static int g_started;
static int g_tapped;
static int g_ime;
static CGContextRef g_ctx;
static void *g_shadow;
static int g_sw;
static int g_sh;
static pthread_mutex_t g_mu = PTHREAD_MUTEX_INITIALIZER;
static int g_touch_ids[16];
static __unsafe_unretained UITouch *g_touch_refs[16];

static void *run_app(void *arg)
{
    (void)arg;
    char *argv[2] = {"app", NULL};
    salam_app_main(1, argv);
    exit(0);
    return NULL;
}

static int touch_slot(UITouch *t, int create)
{
    for (int i = 0; i < 16; i++) {
        if (g_touch_refs[i] == t) {
            return i;
        }
    }
    if (!create) {
        return -1;
    }
    for (int i = 0; i < 16; i++) {
        if (g_touch_refs[i] == nil) {
            g_touch_refs[i] = t;
            return i;
        }
    }
    return -1;
}

@interface SalamView : UIView <UIKeyInput>
@end

@implementation SalamView

- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        self.multipleTouchEnabled = YES;
        self.opaque = YES;
        self.contentScaleFactor = UIScreen.mainScreen.scale;
    }
    return self;
}

- (BOOL)canBecomeFirstResponder
{
    return YES;
}

- (BOOL)hasText
{
    return YES;
}

- (void)insertText:(NSString *)text
{
    if ([text isEqualToString:@"\n"]) {
        salam_host_post(SALAM_EV_KEY, 1, 66, 0, 0, 0, NULL);
        salam_host_post(SALAM_EV_KEY, 0, 66, 0, 0, 0, NULL);
        return;
    }
    const char *u = text.UTF8String;
    size_t n = u != NULL ? strlen(u) : 0;
    size_t i = 0;
    while (i < n) {
        size_t len = n - i;
        if (len >= SALAM_HOST_TCAP) {
            len = SALAM_HOST_TCAP - 1;
            while (len > 0 && (((unsigned char)u[i + len]) & 0xC0) == 0x80) {
                len--;
            }
            if (len == 0) {
                break;
            }
        }
        char buf[SALAM_HOST_TCAP];
        memcpy(buf, u + i, len);
        buf[len] = 0;
        salam_host_post(SALAM_EV_TEXT, 0, 0, 0, 0, 0, buf);
        i += len;
    }
}

static int hid_to_key(UIKeyboardHIDUsage u)
{
    switch (u) {
    case UIKeyboardHIDUsageKeyboardReturnOrEnter:
        return 66;
    case UIKeyboardHIDUsageKeyboardDeleteOrBackspace:
        return 67;
    case UIKeyboardHIDUsageKeyboardDeleteForward:
        return 112;
    case UIKeyboardHIDUsageKeyboardTab:
        return 61;
    case UIKeyboardHIDUsageKeyboardEscape:
        return 111;
    case UIKeyboardHIDUsageKeyboardUpArrow:
        return 19;
    case UIKeyboardHIDUsageKeyboardDownArrow:
        return 20;
    case UIKeyboardHIDUsageKeyboardLeftArrow:
        return 21;
    case UIKeyboardHIDUsageKeyboardRightArrow:
        return 22;
    case UIKeyboardHIDUsageKeyboardHome:
        return 122;
    case UIKeyboardHIDUsageKeyboardEnd:
        return 123;
    case UIKeyboardHIDUsageKeyboardPageUp:
        return 92;
    case UIKeyboardHIDUsageKeyboardPageDown:
        return 93;
    default:
        return 0;
    }
}

- (void)sendPresses:(NSSet<UIPress *> *)presses down:(int)down
{
    for (UIPress *p in presses) {
        int code = p.key != nil ? hid_to_key(p.key.keyCode) : 0;
        if (code != 0) {
            salam_host_post(SALAM_EV_KEY, down, code, 0, 0, 0, NULL);
        }
    }
}

- (void)pressesBegan:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event
{
    [self sendPresses:presses down:1];
    [super pressesBegan:presses withEvent:event];
}

- (void)pressesEnded:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event
{
    [self sendPresses:presses down:0];
    [super pressesEnded:presses withEvent:event];
}

- (void)pressesCancelled:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event
{
    [self sendPresses:presses down:0];
    [super pressesCancelled:presses withEvent:event];
}

- (void)deleteBackward
{
    salam_host_post(SALAM_EV_KEY, 1, 67, 0, 0, 0, NULL);
    salam_host_post(SALAM_EV_KEY, 0, 67, 0, 0, 0, NULL);
}

- (UIKeyboardType)keyboardType
{
    return UIKeyboardTypeDefault;
}

- (UITextAutocorrectionType)autocorrectionType
{
    return UITextAutocorrectionTypeNo;
}

- (void)sendTouches:(NSSet<UITouch *> *)touches action:(int)action
{
    CGFloat s = self.contentScaleFactor;
    for (UITouch *t in touches) {
        int slot = touch_slot(t, action == 0);
        if (slot < 0) {
            continue;
        }
        CGPoint p = [t locationInView:self];
        salam_host_post(SALAM_EV_POINTER, action, slot, p.x * s, p.y * s, 0, NULL);
        if (action == 1) {
            g_touch_refs[slot] = nil;
        }
    }
}

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    [self sendTouches:touches action:0];
}

- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    [self sendTouches:touches action:2];
}

- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    [self sendTouches:touches action:1];
}

- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    [self sendTouches:touches action:1];
}

- (void)reportInsets
{
    UIEdgeInsets i = self.safeAreaInsets;
    CGFloat s = self.contentScaleFactor;
    salam_host_post(SALAM_EV_INSETS, i.top * s, i.right * s, i.bottom * s, i.left * s, g_kb * s, NULL);
}

- (void)safeAreaInsetsDidChange
{
    [super safeAreaInsetsDidChange];
    [self reportInsets];
}

- (void)layoutSubviews
{
    [super layoutSubviews];
    CGFloat s = self.contentScaleFactor;
    int w = (int)(self.bounds.size.width * s);
    int h = (int)(self.bounds.size.height * s);
    if (w > 0 && h > 0) {
        salam_host_set_surface(w, h, s);
        salam_host_post(SALAM_EV_RESIZE, w, h, s, 0, 0, NULL);
        salam_host_post(SALAM_EV_SCHEME, self.traitCollection.userInterfaceStyle == UIUserInterfaceStyleDark ? 1 : 0, 0, 0, 0, 0, NULL);
        [self reportInsets];
        if (!g_started) {
            g_started = 1;
            pthread_create(&g_thread, NULL, run_app, NULL);
        }
        NSArray<NSString *> *args = NSProcessInfo.processInfo.arguments;
        NSUInteger at = [args indexOfObject:@"-salam-tap"];
        if (!g_tapped && at != NSNotFound && at + 1 < args.count) {
            g_tapped = 1;
            NSArray<NSString *> *xy = [args[at + 1] componentsSeparatedByString:@","];
            if (xy.count == 2) {
                double px = xy[0].doubleValue * s;
                double py = xy[1].doubleValue * s;
                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
                    salam_host_post(SALAM_EV_POINTER, 0, 0, px, py, 0, NULL);
                    salam_host_post(SALAM_EV_POINTER, 1, 0, px, py, 0, NULL);
                });
            }
        }
    }
}

- (void)traitCollectionDidChange:(UITraitCollection *)previous
{
    [super traitCollectionDidChange:previous];
    salam_host_post(SALAM_EV_SCHEME, self.traitCollection.userInterfaceStyle == UIUserInterfaceStyleDark ? 1 : 0, 0, 0, 0, 0, NULL);
}

@end

@interface SalamController : UIViewController
@end

@implementation SalamController

- (void)loadView
{
    SalamView *v = [[SalamView alloc] initWithFrame:UIScreen.mainScreen.bounds];
    v.backgroundColor = UIColor.whiteColor;
    g_view = v;
    self.view = v;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    NSNotificationCenter *c = NSNotificationCenter.defaultCenter;
    [c addObserver:self selector:@selector(keyboardChanged:) name:UIKeyboardWillChangeFrameNotification object:nil];
    [c addObserver:self selector:@selector(keyboardHidden:) name:UIKeyboardWillHideNotification object:nil];
    [c addObserver:self selector:@selector(lifecycle:) name:UIApplicationWillResignActiveNotification object:nil];
    [c addObserver:self selector:@selector(lifecycle:) name:UIApplicationDidBecomeActiveNotification object:nil];
    [c addObserver:self selector:@selector(lifecycle:) name:UIApplicationDidEnterBackgroundNotification object:nil];
    [c addObserver:self selector:@selector(lifecycle:) name:UIApplicationDidReceiveMemoryWarningNotification object:nil];
}

- (BOOL)prefersStatusBarHidden
{
    return NO;
}

- (void)keyboardChanged:(NSNotification *)n
{
    CGRect f = [n.userInfo[UIKeyboardFrameEndUserInfoKey] CGRectValue];
    CGRect local = [g_view convertRect:f fromView:nil];
    CGFloat kb = MAX(0, g_view.bounds.size.height - local.origin.y);
    g_kb = kb - g_view.safeAreaInsets.bottom > 0 ? kb : 0;
    [g_view reportInsets];
}

- (void)keyboardHidden:(NSNotification *)n
{
    g_kb = 0;
    [g_view reportInsets];
}

- (void)lifecycle:(NSNotification *)n
{
    if ([n.name isEqualToString:UIApplicationWillResignActiveNotification]) {
        salam_host_post(SALAM_EV_LIFECYCLE, 0, 0, 0, 0, 0, NULL);
    } else if ([n.name isEqualToString:UIApplicationDidBecomeActiveNotification]) {
        salam_host_post(SALAM_EV_LIFECYCLE, 1, 0, 0, 0, 0, NULL);
    } else if ([n.name isEqualToString:UIApplicationDidEnterBackgroundNotification]) {
        salam_host_post(SALAM_EV_LIFECYCLE, 2, 0, 0, 0, 0, NULL);
        __block UIBackgroundTaskIdentifier task = UIBackgroundTaskInvalid;
        void (^end)(void) = ^{
            if (task != UIBackgroundTaskInvalid) {
                [UIApplication.sharedApplication endBackgroundTask:task];
                task = UIBackgroundTaskInvalid;
            }
        };
        task = [UIApplication.sharedApplication beginBackgroundTaskWithExpirationHandler:end];
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2 * NSEC_PER_SEC)), dispatch_get_main_queue(), end);
    } else {
        salam_host_post(SALAM_EV_LIFECYCLE, 3, 0, 0, 0, 0, NULL);
    }
}

@end

@interface SalamSceneDelegate : UIResponder <UIWindowSceneDelegate>
@property(strong, nonatomic) UIWindow *window;
@end

@implementation SalamSceneDelegate

- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options
{
    if (![scene isKindOfClass:UIWindowScene.class]) {
        return;
    }
    self.window = [[UIWindow alloc] initWithWindowScene:(UIWindowScene *)scene];
    self.window.rootViewController = [SalamController new];
    [self.window makeKeyAndVisible];
}

@end

@interface SalamDelegate : UIResponder <UIApplicationDelegate>
@end

@implementation SalamDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)options
{
    NSString *docs = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
    setenv("HOME", docs.UTF8String, 1);
    setenv("TMPDIR", NSTemporaryDirectory().UTF8String, 1);
    NSString *fonts = [NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"fonts"];
    setenv("SALAM_FONT_DIR", fonts.UTF8String, 1);
    salam_device_init();
    return YES;
}

- (UISceneConfiguration *)application:(UIApplication *)application configurationForConnectingSceneSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options
{
    UISceneConfiguration *c = [[UISceneConfiguration alloc] initWithName:@"Default" sessionRole:session.role];
    c.delegateClass = SalamSceneDelegate.class;
    return c;
}

@end

int salam_host_blit(const void *pixels, int bw, int bh, int x, int y, int w, int h)
{
    if (pixels == NULL || bw <= 0 || bh <= 0) {
        return 0;
    }
    pthread_mutex_lock(&g_mu);
    if (g_ctx == NULL || g_sw != bw || g_sh != bh) {
        if (g_ctx != NULL) {
            CGContextRelease(g_ctx);
            free(g_shadow);
        }
        g_shadow = calloc((size_t)bw * bh, 4);
        CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
        g_ctx = CGBitmapContextCreate(g_shadow, bw, bh, 8, (size_t)bw * 4, cs, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little);
        CGColorSpaceRelease(cs);
        g_sw = bw;
        g_sh = bh;
        x = 0;
        y = 0;
        w = bw;
        h = bh;
    }
    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = x + w > bw ? bw : x + w;
    int y1 = y + h > bh ? bh : y + h;
    for (int row = y0; row < y1; row++) {
        memcpy((char *)g_shadow + ((size_t)row * bw + x0) * 4, (const char *)pixels + ((size_t)row * bw + x0) * 4, (size_t)(x1 - x0) * 4);
    }
    CGImageRef img = CGBitmapContextCreateImage(g_ctx);
    pthread_mutex_unlock(&g_mu);
    dispatch_async(dispatch_get_main_queue(), ^{
        g_view.layer.contents = (__bridge id)img;
        CGImageRelease(img);
    });
    return 1;
}

void salam_host_ime(int flag)
{
    dispatch_async(dispatch_get_main_queue(), ^{
        if (flag) {
            [g_view becomeFirstResponder];
        } else {
            [g_view resignFirstResponder];
        }
    });
}

int salam_host_open_url(const char *url)
{
    if (url == NULL) {
        return 0;
    }
    NSString *str = [NSString stringWithUTF8String:url];
    NSURL *u = str != nil ? [NSURL URLWithString:str] : nil;
    if (u == nil) {
        return 0;
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        [UIApplication.sharedApplication openURL:u options:@{} completionHandler:nil];
    });
    return 1;
}

static int g_photo_state;
static char g_photo_path[1024];
static int g_loc_state;
static double g_loc_val[3];
static pthread_mutex_t g_dev_mu = PTHREAD_MUTEX_INITIALIZER;

static void run_main_sync(void (^fn)(void))
{
    if ([NSThread isMainThread]) {
        fn();
    } else {
        dispatch_sync(dispatch_get_main_queue(), fn);
    }
}

static UIViewController *top_controller(void)
{
    UIViewController *c = g_view.window.rootViewController;
    while (c.presentedViewController != nil) {
        c = c.presentedViewController;
    }
    return c;
}

static void photo_finish(NSString *path)
{
    pthread_mutex_lock(&g_dev_mu);
    g_photo_path[0] = 0;
    if (path.length > 0) {
        strncpy(g_photo_path, path.UTF8String, sizeof g_photo_path - 1);
        g_photo_path[sizeof g_photo_path - 1] = 0;
        g_photo_state = 2;
    } else {
        g_photo_state = 3;
    }
    pthread_mutex_unlock(&g_dev_mu);
    salam_host_post(SALAM_EV_DEVICE, 3, 0, 0, 0, 0, NULL);
}

static NSString *save_jpeg(UIImage *img, NSString *prefix)
{
    NSData *d = img != nil ? UIImageJPEGRepresentation(img, 0.92) : nil;
    if (d.length == 0) {
        return nil;
    }
    NSString *dir = [NSTemporaryDirectory() stringByAppendingPathComponent:@"photos"];
    [NSFileManager.defaultManager createDirectoryAtPath:dir withIntermediateDirectories:YES attributes:nil error:nil];
    NSString *path = [dir stringByAppendingPathComponent:[NSString stringWithFormat:@"%@_%lld.jpg", prefix, (long long)(NSDate.date.timeIntervalSince1970 * 1000)]];
    return [d writeToFile:path atomically:YES] ? path : nil;
}

@interface SalamDevice : NSObject <CLLocationManagerDelegate, UNUserNotificationCenterDelegate, PHPickerViewControllerDelegate, UIImagePickerControllerDelegate, UINavigationControllerDelegate>
@property(strong, nonatomic) CLLocationManager *lm;
@end

static SalamDevice *g_dev;

@implementation SalamDevice

- (CLLocationManager *)manager
{
    if (self.lm == nil) {
        self.lm = [CLLocationManager new];
        self.lm.delegate = self;
        self.lm.desiredAccuracy = kCLLocationAccuracyBest;
    }
    return self.lm;
}

- (void)locationManagerDidChangeAuthorization:(CLLocationManager *)manager
{
    if (manager.authorizationStatus != kCLAuthorizationStatusNotDetermined) {
        salam_host_post(SALAM_EV_DEVICE, 1, 0, 0, 0, 0, NULL);
    }
}

- (void)locationManager:(CLLocationManager *)manager didUpdateLocations:(NSArray<CLLocation *> *)locations
{
    CLLocation *l = locations.lastObject;
    pthread_mutex_lock(&g_dev_mu);
    g_loc_state = l != nil ? 2 : 3;
    g_loc_val[0] = l != nil ? l.coordinate.latitude : 0;
    g_loc_val[1] = l != nil ? l.coordinate.longitude : 0;
    g_loc_val[2] = l != nil && l.horizontalAccuracy >= 0 ? l.horizontalAccuracy : 0;
    pthread_mutex_unlock(&g_dev_mu);
    salam_host_post(SALAM_EV_DEVICE, 2, 0, 0, 0, 0, NULL);
}

- (void)locationManager:(CLLocationManager *)manager didFailWithError:(NSError *)error
{
    pthread_mutex_lock(&g_dev_mu);
    g_loc_state = 3;
    pthread_mutex_unlock(&g_dev_mu);
    salam_host_post(SALAM_EV_DEVICE, 2, 0, 0, 0, 0, NULL);
}

- (void)userNotificationCenter:(UNUserNotificationCenter *)center willPresentNotification:(UNNotification *)notification withCompletionHandler:(void (^)(UNNotificationPresentationOptions))handler
{
    handler(UNNotificationPresentationOptionBanner | UNNotificationPresentationOptionList | UNNotificationPresentationOptionSound);
}

- (void)picker:(PHPickerViewController *)picker didFinishPicking:(NSArray<PHPickerResult *> *)results
{
    [picker dismissViewControllerAnimated:YES completion:nil];
    PHPickerResult *r = results.firstObject;
    if (r == nil || ![r.itemProvider canLoadObjectOfClass:UIImage.class]) {
        photo_finish(nil);
        return;
    }
    [r.itemProvider loadObjectOfClass:UIImage.class completionHandler:^(id<NSItemProviderReading> obj, NSError *err) {
        photo_finish([obj isKindOfClass:UIImage.class] ? save_jpeg((UIImage *)obj, @"pick") : nil);
    }];
}

- (void)imagePickerController:(UIImagePickerController *)picker didFinishPickingMediaWithInfo:(NSDictionary<UIImagePickerControllerInfoKey, id> *)info
{
    [picker dismissViewControllerAnimated:YES completion:nil];
    UIImage *img = info[UIImagePickerControllerOriginalImage];
    photo_finish(save_jpeg(img, @"cap"));
}

- (void)imagePickerControllerDidCancel:(UIImagePickerController *)picker
{
    [picker dismissViewControllerAnimated:YES completion:nil];
    photo_finish(nil);
}

@end

static void salam_device_init(void)
{
    g_dev = [SalamDevice new];
    UNUserNotificationCenter.currentNotificationCenter.delegate = g_dev;
}

static int av_state(AVMediaType t)
{
    switch ([AVCaptureDevice authorizationStatusForMediaType:t]) {
    case AVAuthorizationStatusNotDetermined:
        return 0;
    case AVAuthorizationStatusAuthorized:
        return 1;
    default:
        return 2;
    }
}

static int location_auth_state(void)
{
    __block int r = 0;
    run_main_sync(^{
        switch ([g_dev manager].authorizationStatus) {
        case kCLAuthorizationStatusNotDetermined:
            r = 0;
            break;
        case kCLAuthorizationStatusAuthorizedAlways:
        case kCLAuthorizationStatusAuthorizedWhenInUse:
            r = 1;
            break;
        default:
            r = 2;
            break;
        }
    });
    return r;
}

static int notification_state(void)
{
    __block int r = 0;
    dispatch_semaphore_t sem = dispatch_semaphore_create(0);
    [UNUserNotificationCenter.currentNotificationCenter getNotificationSettingsWithCompletionHandler:^(UNNotificationSettings *s) {
        switch (s.authorizationStatus) {
        case UNAuthorizationStatusNotDetermined:
            r = 0;
            break;
        case UNAuthorizationStatusDenied:
            r = 2;
            break;
        default:
            r = 1;
            break;
        }
        dispatch_semaphore_signal(sem);
    }];
    dispatch_semaphore_wait(sem, dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2 * NSEC_PER_SEC)));
    return r;
}

int salam_host_perm_state(const char *name)
{
    if (name == NULL) {
        return 0;
    }
    if (strcmp(name, "camera") == 0) {
        return av_state(AVMediaTypeVideo);
    }
    if (strcmp(name, "microphone") == 0) {
        return av_state(AVMediaTypeAudio);
    }
    if (strcmp(name, "location") == 0) {
        return location_auth_state();
    }
    if (strcmp(name, "notifications") == 0) {
        return notification_state();
    }
    return 2;
}

int salam_host_perm_request(const char *name)
{
    if (name == NULL) {
        return 0;
    }
    int known = strcmp(name, "camera") == 0 || strcmp(name, "microphone") == 0 || strcmp(name, "location") == 0 || strcmp(name, "notifications") == 0;
    if (!known) {
        return 0;
    }
    if (salam_host_perm_state(name) != 0) {
        salam_host_post(SALAM_EV_DEVICE, 1, 0, 0, 0, 0, NULL);
        return 1;
    }
    if (strcmp(name, "camera") == 0 || strcmp(name, "microphone") == 0) {
        AVMediaType t = strcmp(name, "camera") == 0 ? AVMediaTypeVideo : AVMediaTypeAudio;
        [AVCaptureDevice requestAccessForMediaType:t completionHandler:^(BOOL granted) {
            salam_host_post(SALAM_EV_DEVICE, 1, 0, 0, 0, 0, NULL);
        }];
    } else if (strcmp(name, "location") == 0) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [[g_dev manager] requestWhenInUseAuthorization];
        });
    } else {
        [UNUserNotificationCenter.currentNotificationCenter requestAuthorizationWithOptions:(UNAuthorizationOptionAlert | UNAuthorizationOptionSound | UNAuthorizationOptionBadge)
                                                                          completionHandler:^(BOOL granted, NSError *err) {
                                                                              salam_host_post(SALAM_EV_DEVICE, 1, 0, 0, 0, 0, NULL);
                                                                          }];
    }
    return 1;
}

int salam_host_photo_request(int camera)
{
    pthread_mutex_lock(&g_dev_mu);
    g_photo_state = 1;
    pthread_mutex_unlock(&g_dev_mu);
    if (camera && (![UIImagePickerController isSourceTypeAvailable:UIImagePickerControllerSourceTypeCamera] || av_state(AVMediaTypeVideo) == 2)) {
        pthread_mutex_lock(&g_dev_mu);
        g_photo_state = 3;
        pthread_mutex_unlock(&g_dev_mu);
        return 0;
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        UIViewController *top = top_controller();
        if (top == nil) {
            photo_finish(nil);
            return;
        }
        if (camera) {
            UIImagePickerController *p = [UIImagePickerController new];
            p.sourceType = UIImagePickerControllerSourceTypeCamera;
            p.delegate = g_dev;
            [top presentViewController:p animated:YES completion:nil];
        } else {
            PHPickerConfiguration *cfg = [PHPickerConfiguration new];
            cfg.filter = [PHPickerFilter imagesFilter];
            cfg.selectionLimit = 1;
            PHPickerViewController *p = [[PHPickerViewController alloc] initWithConfiguration:cfg];
            p.delegate = g_dev;
            [top presentViewController:p animated:YES completion:nil];
        }
    });
    return 1;
}

int salam_host_photo_state(void)
{
    pthread_mutex_lock(&g_dev_mu);
    int s = g_photo_state;
    pthread_mutex_unlock(&g_dev_mu);
    return s;
}

const char *salam_host_photo_path(void)
{
    static char copy[1024];
    pthread_mutex_lock(&g_dev_mu);
    memcpy(copy, g_photo_path, sizeof copy);
    pthread_mutex_unlock(&g_dev_mu);
    return copy;
}

int salam_host_location_request(void)
{
    if (location_auth_state() != 1) {
        pthread_mutex_lock(&g_dev_mu);
        g_loc_state = 3;
        pthread_mutex_unlock(&g_dev_mu);
        return 0;
    }
    pthread_mutex_lock(&g_dev_mu);
    g_loc_state = 1;
    pthread_mutex_unlock(&g_dev_mu);
    dispatch_async(dispatch_get_main_queue(), ^{
        [[g_dev manager] requestLocation];
    });
    return 1;
}

int salam_host_location_get(double *out)
{
    pthread_mutex_lock(&g_dev_mu);
    int s = g_loc_state;
    out[0] = g_loc_val[0];
    out[1] = g_loc_val[1];
    out[2] = g_loc_val[2];
    pthread_mutex_unlock(&g_dev_mu);
    return s;
}

int salam_host_notify(int id, const char *title, const char *body, int delay_ms)
{
    if (notification_state() != 1) {
        return 0;
    }
    UNMutableNotificationContent *c = [UNMutableNotificationContent new];
    c.title = title != NULL ? [NSString stringWithUTF8String:title] ?: @"" : @"";
    c.body = body != NULL ? [NSString stringWithUTF8String:body] ?: @"" : @"";
    c.sound = UNNotificationSound.defaultSound;
    UNNotificationTrigger *trigger = nil;
    if (delay_ms > 0) {
        NSTimeInterval secs = delay_ms / 1000.0;
        trigger = [UNTimeIntervalNotificationTrigger triggerWithTimeInterval:secs < 1 ? 1 : secs repeats:NO];
    }
    NSString *ident = [NSString stringWithFormat:@"salam.%d", id];
    UNNotificationRequest *r = [UNNotificationRequest requestWithIdentifier:ident content:c trigger:trigger];
    [UNUserNotificationCenter.currentNotificationCenter addNotificationRequest:r withCompletionHandler:nil];
    return 1;
}

int salam_host_notify_cancel(int id)
{
    NSString *ident = [NSString stringWithFormat:@"salam.%d", id];
    UNUserNotificationCenter *n = UNUserNotificationCenter.currentNotificationCenter;
    [n removePendingNotificationRequestsWithIdentifiers:@[ ident ]];
    [n removeDeliveredNotificationsWithIdentifiers:@[ ident ]];
    return 1;
}

int salam_host_vibrate(int ms)
{
    if (ms <= 0) {
        return 0;
    }
    AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
    return 1;
}

int salam_host_haptic(int kind)
{
    if (kind < 0 || kind > 3) {
        return 0;
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        if (kind <= 1) {
            UIImpactFeedbackGenerator *g = [[UIImpactFeedbackGenerator alloc] initWithStyle:kind == 0 ? UIImpactFeedbackStyleLight : UIImpactFeedbackStyleHeavy];
            [g prepare];
            [g impactOccurred];
        } else {
            UINotificationFeedbackGenerator *g = [UINotificationFeedbackGenerator new];
            [g prepare];
            [g notificationOccurred:kind == 2 ? UINotificationFeedbackTypeSuccess : UINotificationFeedbackTypeError];
        }
    });
    return 1;
}

int salam_host_share(const char *text, const char *url)
{
    NSMutableArray *items = [NSMutableArray new];
    NSString *t = text != NULL ? [NSString stringWithUTF8String:text] : nil;
    NSString *u = url != NULL ? [NSString stringWithUTF8String:url] : nil;
    if (t.length > 0) {
        [items addObject:t];
    }
    NSURL *link = u.length > 0 ? [NSURL URLWithString:u] : nil;
    if (link != nil) {
        [items addObject:link];
    }
    if (items.count == 0) {
        return 0;
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        UIViewController *top = top_controller();
        if (top == nil) {
            return;
        }
        UIActivityViewController *a = [[UIActivityViewController alloc] initWithActivityItems:items applicationActivities:nil];
        a.popoverPresentationController.sourceView = g_view;
        a.popoverPresentationController.sourceRect = CGRectMake(g_view.bounds.size.width / 2, g_view.bounds.size.height / 2, 1, 1);
        [top presentViewController:a animated:YES completion:nil];
    });
    return 1;
}

int salam_host_clip_set(const char *text)
{
    if (text == NULL) {
        return 0;
    }
    NSString *s = [NSString stringWithUTF8String:text];
    dispatch_sync(dispatch_get_main_queue(), ^{
        UIPasteboard.generalPasteboard.string = s;
    });
    return 1;
}

const char *salam_host_clip_get(void)
{
    static char buf[8192];
    __block NSString *s = nil;
    dispatch_sync(dispatch_get_main_queue(), ^{
        s = UIPasteboard.generalPasteboard.string;
    });
    buf[0] = 0;
    if (s != nil) {
        strncpy(buf, s.UTF8String, sizeof buf - 1);
        buf[sizeof buf - 1] = 0;
    }
    return buf;
}

int main(int argc, char *argv[])
{
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([SalamDelegate class]));
    }
}
