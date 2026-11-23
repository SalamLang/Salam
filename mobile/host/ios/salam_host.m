#import <UIKit/UIKit.h>
#import <CoreGraphics/CoreGraphics.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "../common/salam_host_common.h"

extern int salam_app_main(int argc, char **argv);

@class SalamView;

static SalamView *g_view;
static UIKeyboardType g_unused;
static pthread_t g_thread;
static int g_started;
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
    salam_host_post(SALAM_EV_TEXT, 0, 0, 0, 0, 0, text.UTF8String);
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

- (void)reportInsets:(CGFloat)keyboard
{
    UIEdgeInsets i = self.safeAreaInsets;
    CGFloat s = self.contentScaleFactor;
    salam_host_post(SALAM_EV_INSETS, i.top * s, i.right * s, i.bottom * s, i.left * s, keyboard * s, NULL);
}

- (void)safeAreaInsetsDidChange
{
    [super safeAreaInsetsDidChange];
    [self reportInsets:0];
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
        [self reportInsets:0];
        if (!g_started) {
            g_started = 1;
            pthread_create(&g_thread, NULL, run_app, NULL);
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
    [g_view reportInsets:kb - g_view.safeAreaInsets.bottom > 0 ? kb : 0];
}

- (void)keyboardHidden:(NSNotification *)n
{
    [g_view reportInsets:0];
}

- (void)lifecycle:(NSNotification *)n
{
    if ([n.name isEqualToString:UIApplicationWillResignActiveNotification]) {
        salam_host_post(SALAM_EV_LIFECYCLE, 0, 0, 0, 0, 0, NULL);
    } else if ([n.name isEqualToString:UIApplicationDidBecomeActiveNotification]) {
        salam_host_post(SALAM_EV_LIFECYCLE, 1, 0, 0, 0, 0, NULL);
    } else if ([n.name isEqualToString:UIApplicationDidEnterBackgroundNotification]) {
        salam_host_post(SALAM_EV_LIFECYCLE, 2, 0, 0, 0, 0, NULL);
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
    NSURL *u = [NSURL URLWithString:[NSString stringWithUTF8String:url]];
    if (u == nil) {
        return 0;
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        [UIApplication.sharedApplication openURL:u options:@{} completionHandler:nil];
    });
    return 1;
}

int salam_host_clip_set(const char *text)
{
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
