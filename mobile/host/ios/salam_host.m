#import <UIKit/UIKit.h>
#import <CoreGraphics/CoreGraphics.h>
#import <AVFoundation/AVFoundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import <CoreLocation/CoreLocation.h>
#import <PhotosUI/PhotosUI.h>
#import <UserNotifications/UserNotifications.h>
#import <Security/Security.h>
#import <Network/Network.h>
#import <LocalAuthentication/LocalAuthentication.h>
#include <sys/utsname.h>
#include <execinfo.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../common/salam_host_common.h"

extern int salam_app_main(int argc, char **argv);

@class SalamView;

static SalamView *g_view;
static void salam_device_init(void);
static CGFloat g_kb;
static pthread_t g_thread;
static int g_started;
static int g_tapped;
static void (^g_first_frame)(void);
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
    stack_t st;
    st.ss_sp = malloc(65536);
    st.ss_size = 65536;
    st.ss_flags = 0;
    if (st.ss_sp != NULL) {
        sigaltstack(&st, NULL);
    }
    salam_app_main(1, argv);
    exit(0);
    return NULL;
}

static int g_ime_rect[4];
static int g_ime_cfg;

static void post_text(NSString *text, int commit)
{
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
        salam_host_post(SALAM_EV_TEXT, commit, 0, 0, 0, 0, buf);
        i += len;
    }
}

static void post_preedit(NSString *text, NSUInteger caret)
{
    NSString *head = caret <= text.length ? [text substringToIndex:caret] : text;
    const char *u = text.UTF8String;
    char buf[SALAM_HOST_TCAP];
    size_t n = u != NULL ? strlen(u) : 0;
    if (n >= SALAM_HOST_TCAP) {
        n = SALAM_HOST_TCAP - 1;
        while (n > 0 && (((unsigned char)u[n]) & 0xC0) == 0x80) {
            n--;
        }
    }
    if (n > 0) {
        memcpy(buf, u, n);
    }
    buf[n] = 0;
    size_t hb = strlen(head.UTF8String);
    salam_host_post(SALAM_EV_PREEDIT, hb > n ? n : hb, 0, 0, 0, 0, buf);
}

@interface SalamPos : UITextPosition
@property(nonatomic) NSInteger off;
@end

@implementation SalamPos
@end

@interface SalamRange : UITextRange
@property(strong, nonatomic) SalamPos *from;
@property(strong, nonatomic) SalamPos *to;
@end

@implementation SalamRange
- (UITextPosition *)start
{
    return self.from;
}
- (UITextPosition *)end
{
    return self.to;
}
- (BOOL)isEmpty
{
    return self.from.off == self.to.off;
}
@end

static SalamPos *make_pos(NSInteger off)
{
    SalamPos *p = [SalamPos new];
    p.off = off < 0 ? 0 : off;
    return p;
}

static SalamRange *make_range(NSInteger a, NSInteger b)
{
    SalamRange *r = [SalamRange new];
    r.from = make_pos(a < b ? a : b);
    r.to = make_pos(a < b ? b : a);
    return r;
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

static void run_script(id view, NSString *script, CGFloat scale);

@interface SalamView : UIView <UITextInput>
@property(copy, nonatomic) NSString *marked;
@property(nonatomic) NSRange sel;
@end

@implementation SalamView
@synthesize inputDelegate;
@synthesize tokenizer;
@synthesize markedTextStyle;

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
    BOOL composing = self.marked.length > 0;
    if (composing) {
        [self.inputDelegate textWillChange:self];
        self.marked = nil;
        self.sel = NSMakeRange(0, 0);
        [self.inputDelegate textDidChange:self];
        if ([text isEqualToString:@"\n"]) {
            return;
        }
    }
    if ([text isEqualToString:@"\n"]) {
        salam_host_post(SALAM_EV_KEY, 1, 66, 0, 0, 0, NULL);
        salam_host_post(SALAM_EV_KEY, 0, 66, 0, 0, 0, NULL);
        return;
    }
    post_text(text, composing ? 1 : 0);
}

- (NSInteger)docLength
{
    return (NSInteger)self.marked.length;
}

- (UITextRange *)selectedTextRange
{
    return make_range((NSInteger)self.sel.location, (NSInteger)(self.sel.location + self.sel.length));
}

- (void)setSelectedTextRange:(UITextRange *)r
{
    SalamRange *sr = (SalamRange *)r;
    if (![sr isKindOfClass:SalamRange.class]) {
        return;
    }
    NSInteger len = [self docLength];
    NSInteger a = MIN(sr.from.off, len);
    NSInteger b = MIN(sr.to.off, len);
    self.sel = NSMakeRange((NSUInteger)a, (NSUInteger)(b - a));
}

- (UITextRange *)markedTextRange
{
    return self.marked.length > 0 ? make_range(0, [self docLength]) : nil;
}

- (id<UITextInputTokenizer>)tokenizer
{
    if (tokenizer == nil) {
        tokenizer = [[UITextInputStringTokenizer alloc] initWithTextInput:self];
    }
    return tokenizer;
}

- (UITextPosition *)beginningOfDocument
{
    return make_pos(0);
}

- (UITextPosition *)endOfDocument
{
    return make_pos([self docLength]);
}

- (NSString *)textInRange:(UITextRange *)r
{
    SalamRange *sr = (SalamRange *)r;
    NSString *doc = self.marked ?: @"";
    if (![sr isKindOfClass:SalamRange.class]) {
        return @"";
    }
    NSInteger a = MIN(sr.from.off, (NSInteger)doc.length);
    NSInteger b = MIN(sr.to.off, (NSInteger)doc.length);
    return [doc substringWithRange:NSMakeRange((NSUInteger)a, (NSUInteger)(b - a))];
}

- (void)replaceRange:(UITextRange *)r withText:(NSString *)text
{
    [self insertText:text];
}

- (void)setMarkedText:(NSString *)text selectedRange:(NSRange)range
{
    [self.inputDelegate textWillChange:self];
    if (text.length == 0) {
        self.marked = nil;
        self.sel = NSMakeRange(0, 0);
        post_preedit(@"", 0);
    } else {
        self.marked = text;
        NSUInteger at = MIN(range.location, text.length);
        self.sel = NSMakeRange(at, 0);
        post_preedit(text, at);
    }
    [self.inputDelegate textDidChange:self];
}

- (void)unmarkText
{
    if (self.marked.length == 0) {
        return;
    }
    NSString *t = self.marked;
    [self.inputDelegate textWillChange:self];
    self.marked = nil;
    self.sel = NSMakeRange(0, 0);
    [self.inputDelegate textDidChange:self];
    post_text(t, 1);
}

- (UITextRange *)textRangeFromPosition:(UITextPosition *)a toPosition:(UITextPosition *)b
{
    return make_range(((SalamPos *)a).off, ((SalamPos *)b).off);
}

- (UITextPosition *)positionFromPosition:(UITextPosition *)p offset:(NSInteger)offset
{
    NSInteger v = ((SalamPos *)p).off + offset;
    if (v < 0 || v > [self docLength]) {
        return nil;
    }
    return make_pos(v);
}

- (UITextPosition *)positionFromPosition:(UITextPosition *)p inDirection:(UITextLayoutDirection)dir offset:(NSInteger)offset
{
    NSInteger d = (dir == UITextLayoutDirectionLeft || dir == UITextLayoutDirectionUp) ? -offset : offset;
    return [self positionFromPosition:p offset:d];
}

- (NSComparisonResult)comparePosition:(UITextPosition *)a toPosition:(UITextPosition *)b
{
    NSInteger x = ((SalamPos *)a).off;
    NSInteger y = ((SalamPos *)b).off;
    return x < y ? NSOrderedAscending : (x > y ? NSOrderedDescending : NSOrderedSame);
}

- (NSInteger)offsetFromPosition:(UITextPosition *)a toPosition:(UITextPosition *)b
{
    return ((SalamPos *)b).off - ((SalamPos *)a).off;
}

- (UITextPosition *)positionWithinRange:(UITextRange *)r farthestInDirection:(UITextLayoutDirection)dir
{
    SalamRange *sr = (SalamRange *)r;
    return (dir == UITextLayoutDirectionLeft || dir == UITextLayoutDirectionUp) ? sr.from : sr.to;
}

- (UITextRange *)characterRangeByExtendingPosition:(UITextPosition *)p inDirection:(UITextLayoutDirection)dir
{
    NSInteger o = ((SalamPos *)p).off;
    return (dir == UITextLayoutDirectionLeft || dir == UITextLayoutDirectionUp) ? make_range(0, o) : make_range(o, [self docLength]);
}

- (NSWritingDirection)baseWritingDirectionForPosition:(UITextPosition *)p inDirection:(UITextStorageDirection)dir
{
    return NSWritingDirectionNatural;
}

- (void)setBaseWritingDirection:(NSWritingDirection)d forRange:(UITextRange *)r
{
}

- (CGRect)imeRect
{
    pthread_mutex_lock(&g_mu);
    CGRect r = CGRectMake(g_ime_rect[0], g_ime_rect[1], g_ime_rect[2] > 0 ? g_ime_rect[2] : 1, g_ime_rect[3] > 0 ? g_ime_rect[3] : 20);
    pthread_mutex_unlock(&g_mu);
    return r;
}

- (CGRect)firstRectForRange:(UITextRange *)r
{
    return [self imeRect];
}

- (CGRect)caretRectForPosition:(UITextPosition *)p
{
    return [self imeRect];
}

- (NSArray<UITextSelectionRect *> *)selectionRectsForRange:(UITextRange *)r
{
    return @[];
}

- (UITextPosition *)closestPositionToPoint:(CGPoint)pt
{
    return make_pos([self docLength]);
}

- (UITextPosition *)closestPositionToPoint:(CGPoint)pt withinRange:(UITextRange *)r
{
    return ((SalamRange *)r).to;
}

- (UITextRange *)characterRangeAtPoint:(CGPoint)pt
{
    return nil;
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
    if (self.marked.length > 0) {
        NSRange last = [self.marked rangeOfComposedCharacterSequenceAtIndex:self.marked.length - 1];
        NSString *rest = [self.marked substringToIndex:last.location];
        [self setMarkedText:rest selectedRange:NSMakeRange(rest.length, 0)];
        return;
    }
    salam_host_post(SALAM_EV_KEY, 1, 67, 0, 0, 0, NULL);
    salam_host_post(SALAM_EV_KEY, 0, 67, 0, 0, 0, NULL);
}

- (int)imeCfg
{
    pthread_mutex_lock(&g_mu);
    int c = g_ime_cfg;
    pthread_mutex_unlock(&g_mu);
    return c;
}

- (UIKeyboardType)keyboardType
{
    int cfg = [self imeCfg];
    switch (cfg & 15) {
    case 1:
        return UIKeyboardTypeEmailAddress;
    case 2:
        return UIKeyboardTypeURL;
    case 3:
        return UIKeyboardTypePhonePad;
    case 4:
        return UIKeyboardTypeNumberPad;
    case 5:
        return UIKeyboardTypeDecimalPad;
    case 7:
        return UIKeyboardTypeWebSearch;
    default:
        return UIKeyboardTypeDefault;
    }
}

- (UIReturnKeyType)returnKeyType
{
    switch (([self imeCfg] >> 4) & 15) {
    case 2:
        return UIReturnKeyDone;
    case 3:
        return UIReturnKeyGo;
    case 4:
        return UIReturnKeyNext;
    case 6:
        return UIReturnKeySearch;
    case 7:
        return UIReturnKeySend;
    default:
        return UIReturnKeyDefault;
    }
}

- (UITextAutocapitalizationType)autocapitalizationType
{
    switch (([self imeCfg] >> 8) & 3) {
    case 1:
        return UITextAutocapitalizationTypeNone;
    case 2:
        return UITextAutocapitalizationTypeWords;
    case 3:
        return UITextAutocapitalizationTypeAllCharacters;
    default:
        return UITextAutocapitalizationTypeSentences;
    }
}

- (UITextAutocorrectionType)autocorrectionType
{
    int fix = ([self imeCfg] >> 10) & 3;
    return fix == 2 ? UITextAutocorrectionTypeYes : (fix == 1 ? UITextAutocorrectionTypeNo : UITextAutocorrectionTypeDefault);
}

- (UITextSpellCheckingType)spellCheckingType
{
    int fix = ([self imeCfg] >> 10) & 3;
    return fix == 1 ? UITextSpellCheckingTypeNo : UITextSpellCheckingTypeDefault;
}

- (BOOL)isSecureTextEntry
{
    return ([self imeCfg] & 15) == 6;
}

- (UITextContentType)textContentType
{
    switch (([self imeCfg] >> 12) & 15) {
    case 1:
        return UITextContentTypeUsername;
    case 2:
        return UITextContentTypePassword;
    case 3:
        return UITextContentTypeNewPassword;
    case 4:
        return UITextContentTypeOneTimeCode;
    case 5:
        return UITextContentTypeEmailAddress;
    case 6:
        return UITextContentTypeTelephoneNumber;
    case 7:
        return UITextContentTypeName;
    default:
        return nil;
    }
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
        if (!g_tapped) {
            g_tapped = 1;
            NSArray<NSString *> *args = NSProcessInfo.processInfo.arguments;
            NSUInteger at = [args indexOfObject:@"-salam-script"];
            NSUInteger tap = [args indexOfObject:@"-salam-tap"];
            if (at != NSNotFound && at + 1 < args.count) {
                NSString *script = args[at + 1];
                g_first_frame = ^{
                    run_script(self, script, s);
                };
            } else if (tap != NSNotFound && tap + 1 < args.count) {
                NSString *script = [@"tap " stringByAppendingString:args[tap + 1]];
                g_first_frame = ^{
                    run_script(self, script, s);
                };
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

static void script_at(double ms, void (^fn)(void))
{
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(ms * NSEC_PER_MSEC)), dispatch_get_main_queue(), fn);
}

static void script_pointer(double ms, int action, int id, double x, double y, double s)
{
    script_at(ms, ^{
        salam_host_post(SALAM_EV_POINTER, action, id, x * s, y * s, 0, NULL);
    });
}

static void run_script(id view, NSString *script, CGFloat s)
{
    double t = 1500;
    for (NSString *raw in [script componentsSeparatedByString:@";"]) {
        NSString *step = [raw stringByTrimmingCharactersInSet:NSCharacterSet.whitespaceCharacterSet];
        NSRange sp = [step rangeOfString:@" "];
        NSString *cmd = sp.location == NSNotFound ? step : [step substringToIndex:sp.location];
        NSString *arg = sp.location == NSNotFound ? @"" : [step substringFromIndex:sp.location + 1];
        NSArray<NSString *> *a = [arg componentsSeparatedByString:@","];
        if ([cmd isEqualToString:@"tap"] && a.count >= 2) {
            script_pointer(t, 0, 0, a[0].doubleValue, a[1].doubleValue, s);
            script_pointer(t + 40, 1, 0, a[0].doubleValue, a[1].doubleValue, s);
            t += 200;
        } else if ([cmd isEqualToString:@"long"] && a.count >= 2) {
            double hold = a.count >= 3 ? a[2].doubleValue : 900;
            script_pointer(t, 0, 0, a[0].doubleValue, a[1].doubleValue, s);
            script_pointer(t + hold, 1, 0, a[0].doubleValue, a[1].doubleValue, s);
            t += hold + 200;
        } else if ([cmd isEqualToString:@"swipe"] && a.count >= 4) {
            double x1 = a[0].doubleValue, y1 = a[1].doubleValue, x2 = a[2].doubleValue, y2 = a[3].doubleValue;
            double dur = a.count >= 5 ? a[4].doubleValue : 300;
            int n = 20;
            script_pointer(t, 0, 0, x1, y1, s);
            for (int i = 1; i <= n; i++) {
                double f = (double)i / n;
                script_pointer(t + dur * f, 2, 0, x1 + (x2 - x1) * f, y1 + (y2 - y1) * f, s);
            }
            script_pointer(t + dur + 16, 1, 0, x2, y2, s);
            t += dur + 200;
        } else if ([cmd isEqualToString:@"pinch"] && a.count >= 3) {
            double cx = a[0].doubleValue, cy = a[1].doubleValue, scale = a[2].doubleValue;
            double d0 = 60;
            double dur = a.count >= 4 ? a[3].doubleValue : 400;
            int n = 20;
            script_pointer(t, 0, 0, cx - d0, cy, s);
            script_pointer(t + 8, 0, 1, cx + d0, cy, s);
            for (int i = 1; i <= n; i++) {
                double f = (double)i / n;
                double d = d0 + (d0 * scale - d0) * f;
                script_pointer(t + 16 + dur * f, 2, 0, cx - d, cy, s);
                script_pointer(t + 16 + dur * f, 2, 1, cx + d, cy, s);
            }
            script_pointer(t + dur + 40, 1, 1, cx + d0 * scale, cy, s);
            script_pointer(t + dur + 48, 1, 0, cx - d0 * scale, cy, s);
            t += dur + 300;
        } else if ([cmd isEqualToString:@"wait"]) {
            t += arg.doubleValue;
        } else if ([cmd isEqualToString:@"type"]) {
            script_at(t, ^{
                [view insertText:arg];
            });
            t += 200;
        } else if ([cmd isEqualToString:@"mark"]) {
            script_at(t, ^{
                [view setMarkedText:arg selectedRange:NSMakeRange(arg.length, 0)];
            });
            t += 200;
        } else if ([cmd isEqualToString:@"unmark"]) {
            script_at(t, ^{
                [view unmarkText];
            });
            t += 200;
        } else if ([cmd isEqualToString:@"back"]) {
            script_at(t, ^{
                [view deleteBackward];
            });
            t += 200;
        }
    }
}

static NSString *salam_background_key(UIUserInterfaceStyle style)
{
    return style == UIUserInterfaceStyleDark ? @"salam.page.bg.dark" : @"salam.page.bg.light";
}

static UIColor *salam_saved_background(UIUserInterfaceStyle style)
{
    NSNumber *n = [NSUserDefaults.standardUserDefaults objectForKey:salam_background_key(style)];
    if (n == nil) {
        return UIColor.systemBackgroundColor;
    }
    uint32_t p = n.unsignedIntValue;
    return [UIColor colorWithRed:((p >> 16) & 0xFF) / 255.0 green:((p >> 8) & 0xFF) / 255.0 blue:(p & 0xFF) / 255.0 alpha:1];
}

static int g_orient = 0;
static int g_immersive = 0;
static int g_bar_style = 0;
static float g_saved_brightness = -1;
static int g_bright_asked = -1;
static __weak UIViewController *g_controller;

@interface SalamController : UIViewController
@end

@implementation SalamController

- (void)loadView
{
    SalamView *v = [[SalamView alloc] initWithFrame:UIScreen.mainScreen.bounds];
    v.backgroundColor = salam_saved_background(UIScreen.mainScreen.traitCollection.userInterfaceStyle);
    g_view = v;
    g_controller = self;
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
    return g_immersive != 0;
}

- (BOOL)prefersHomeIndicatorAutoHidden
{
    return g_immersive != 0;
}

- (UIStatusBarStyle)preferredStatusBarStyle
{
    if (g_bar_style == 1) {
        return UIStatusBarStyleLightContent;
    }
    if (g_bar_style == 2) {
        return UIStatusBarStyleDarkContent;
    }
    return UIStatusBarStyleDefault;
}

- (UIInterfaceOrientationMask)supportedInterfaceOrientations
{
    UIInterfaceOrientationMask all = UIInterfaceOrientationMaskAll;
    NSArray *plist = [NSBundle.mainBundle objectForInfoDictionaryKey:@"UISupportedInterfaceOrientations"];
    if (plist.count > 0) {
        all = 0;
        for (NSString *o in plist) {
            if ([o isEqualToString:@"UIInterfaceOrientationPortrait"]) {
                all |= UIInterfaceOrientationMaskPortrait;
            } else if ([o isEqualToString:@"UIInterfaceOrientationLandscapeLeft"]) {
                all |= UIInterfaceOrientationMaskLandscapeLeft;
            } else if ([o isEqualToString:@"UIInterfaceOrientationLandscapeRight"]) {
                all |= UIInterfaceOrientationMaskLandscapeRight;
            } else if ([o isEqualToString:@"UIInterfaceOrientationPortraitUpsideDown"]) {
                all |= UIInterfaceOrientationMaskPortraitUpsideDown;
            }
        }
    }
    if (g_orient == 1) {
        UIInterfaceOrientationMask m = all & UIInterfaceOrientationMaskPortrait;
        return m != 0 ? m : all;
    }
    if (g_orient == 2) {
        UIInterfaceOrientationMask m = all & UIInterfaceOrientationMaskLandscape;
        return m != 0 ? m : all;
    }
    return all;
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

static char g_launch_url[2048];
static char g_pending_url[2048];
static pthread_mutex_t g_url_mu = PTHREAD_MUTEX_INITIALIZER;

static void url_store(NSURL *u, int launch)
{
    if (u == nil || u.absoluteString.length == 0) {
        return;
    }
    pthread_mutex_lock(&g_url_mu);
    char *dst = launch ? g_launch_url : g_pending_url;
    size_t cap = launch ? sizeof g_launch_url : sizeof g_pending_url;
    strncpy(dst, u.absoluteString.UTF8String, cap - 1);
    dst[cap - 1] = 0;
    pthread_mutex_unlock(&g_url_mu);
    if (!launch) {
        salam_host_post(SALAM_EV_DEVICE, 5, 0, 0, 0, 0, NULL);
    }
}

@interface SalamSceneDelegate : UIResponder <UIWindowSceneDelegate>
@property(strong, nonatomic) UIWindow *window;
@end

@implementation SalamSceneDelegate

- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options
{
    if (![scene isKindOfClass:UIWindowScene.class]) {
        return;
    }
    for (UIOpenURLContext *c in options.URLContexts) {
        url_store(c.URL, 1);
    }
    for (NSUserActivity *a in options.userActivities) {
        if ([a.activityType isEqualToString:NSUserActivityTypeBrowsingWeb]) {
            url_store(a.webpageURL, 1);
        }
    }
    self.window = [[UIWindow alloc] initWithWindowScene:(UIWindowScene *)scene];
    self.window.rootViewController = [SalamController new];
    [self.window makeKeyAndVisible];
}

- (void)scene:(UIScene *)scene openURLContexts:(NSSet<UIOpenURLContext *> *)contexts
{
    for (UIOpenURLContext *c in contexts) {
        url_store(c.URL, 0);
    }
}

- (void)scene:(UIScene *)scene continueUserActivity:(NSUserActivity *)activity
{
    if ([activity.activityType isEqualToString:NSUserActivityTypeBrowsingWeb]) {
        url_store(activity.webpageURL, 0);
    }
}

@end

static char g_crash_path[1100];
static int g_err_orig = 2;
static int g_err_pipe[2];
static char g_panic_msg[256];
static volatile sig_atomic_t g_crashing;
static char g_crash_stack[65536];
static struct sigaction g_crash_old[6];
static const int g_crash_sigs[] = {SIGABRT, SIGSEGV, SIGBUS, SIGFPE, SIGILL, SIGTRAP};

static void *err_reader(void *arg)
{
    char buf[512];
    char line[256];
    size_t n = 0;
    ssize_t got;
    (void)arg;
    while ((got = read(g_err_pipe[0], buf, sizeof buf)) > 0) {
        ssize_t off = 0;
        while (off < got) {
            ssize_t w = write(g_err_orig, buf + off, (size_t)(got - off));
            if (w <= 0) {
                break;
            }
            off += w;
        }
        for (ssize_t i = 0; i < got; i++) {
            if (buf[i] == '\n' || n == sizeof line - 1) {
                line[n] = 0;
                if (strncmp(line, "salam panic:", 12) == 0) {
                    strncpy(g_panic_msg, line, sizeof g_panic_msg - 1);
                    NSLog(@"%s", line);
                }
                n = 0;
            } else {
                line[n++] = buf[i];
            }
        }
    }
    return NULL;
}

static void crash_handler(int sig)
{
    void *frames[48];
    char head[200];
    int fd;
    int n;
    if (g_crashing) {
        _exit(128 + sig);
    }
    g_crashing = 1;
    fd = g_crash_path[0] != 0 ? open(g_crash_path, O_WRONLY | O_CREAT | O_TRUNC, 0600) : -1;
    n = snprintf(head, sizeof head, "fatal signal %d\n%s\nbacktrace:\n", sig, g_panic_msg);
    NSLog(@"salam crash: signal %d %s", sig, g_panic_msg);
    if (fd >= 0) {
        if (write(fd, head, (size_t)n) < 0) {
            n = 0;
        }
    }
    n = backtrace(frames, 48);
    if (fd >= 0) {
        backtrace_symbols_fd(frames, n, fd);
        close(fd);
    }
    backtrace_symbols_fd(frames, n, g_err_orig);
    for (size_t i = 0; i < sizeof g_crash_sigs / sizeof g_crash_sigs[0]; i++) {
        if (g_crash_sigs[i] == sig) {
            sigaction(sig, &g_crash_old[i], NULL);
        }
    }
    if (sig == SIGABRT) {
        raise(sig);
    }
}

static void term_handler(int sig)
{
    (void)sig;
    fflush(NULL);
    _exit(0);
}

static void crash_hook(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = crash_handler;
    sa.sa_flags = SA_ONSTACK;
    sigemptyset(&sa.sa_mask);
    for (size_t i = 0; i < sizeof g_crash_sigs / sizeof g_crash_sigs[0]; i++) {
        struct sigaction cur;
        sigaction(g_crash_sigs[i], NULL, &cur);
        if (cur.sa_handler != crash_handler) {
            g_crash_old[i] = cur;
            sigaction(g_crash_sigs[i], &sa, NULL);
        }
    }
    signal(SIGTERM, term_handler);
}

static void crash_install(NSString *docs)
{
    stack_t st;
    [NSFileManager.defaultManager createDirectoryAtPath:docs withIntermediateDirectories:YES attributes:nil error:nil];
    snprintf(g_crash_path, sizeof g_crash_path, "%s/crash.txt", docs.UTF8String);
    g_err_orig = dup(2);
    if (g_err_orig >= 0 && pipe(g_err_pipe) == 0) {
        pthread_t t;
        dup2(g_err_pipe[1], 2);
        pthread_create(&t, NULL, err_reader, NULL);
        pthread_detach(t);
    } else {
        g_err_orig = 2;
    }
    st.ss_sp = g_crash_stack;
    st.ss_size = sizeof g_crash_stack;
    st.ss_flags = 0;
    sigaltstack(&st, NULL);
    crash_hook();
}

static void crash_show_last(NSString *docs)
{
    NSString *path = [docs stringByAppendingPathComponent:@"crash.txt"];
    NSString *text = [NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:nil];
    if (text == nil) {
        return;
    }
    [NSFileManager.defaultManager moveItemAtPath:path toPath:[docs stringByAppendingPathComponent:@"crash.last.txt"] error:nil];
    NSLog(@"salam previous crash:\n%@", text);
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        UIViewController *root = g_view.window.rootViewController;
        if (root == nil) {
            return;
        }
        NSString *shown = text.length > 600 ? [text substringToIndex:600] : text;
        UIAlertController *a = [UIAlertController alertControllerWithTitle:@"The app closed unexpectedly" message:shown preferredStyle:UIAlertControllerStyleAlert];
        [a addAction:[UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault handler:nil]];
        [root presentViewController:a animated:YES completion:nil];
    });
}

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
    crash_install(docs);
    crash_show_last(docs);
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
    uint32_t page = ((const uint32_t *)g_shadow)[0];
    pthread_mutex_unlock(&g_mu);
    dispatch_async(dispatch_get_main_queue(), ^{
        g_view.layer.contents = (__bridge id)img;
        CGImageRelease(img);
        NSString *key = salam_background_key(g_view.traitCollection.userInterfaceStyle);
        NSNumber *old = [NSUserDefaults.standardUserDefaults objectForKey:key];
        if (old == nil || old.unsignedIntValue != page) {
            [NSUserDefaults.standardUserDefaults setObject:@(page) forKey:key];
            g_view.backgroundColor = salam_saved_background(g_view.traitCollection.userInterfaceStyle);
        }
        static int hooked;
        if (!hooked) {
            hooked = 1;
            crash_hook();
        }
        if (g_first_frame != nil) {
            void (^go)(void) = g_first_frame;
            g_first_frame = nil;
            go();
        }
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

void salam_host_ime_config(int config)
{
    pthread_mutex_lock(&g_mu);
    int changed = config != g_ime_cfg;
    g_ime_cfg = config;
    pthread_mutex_unlock(&g_mu);
    if (!changed) {
        return;
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        if (g_view.isFirstResponder) {
            [g_view reloadInputViews];
        }
    });
}

void salam_host_ime_rect(int x, int y, int w, int h)
{
    pthread_mutex_lock(&g_mu);
    g_ime_rect[0] = x;
    g_ime_rect[1] = y;
    g_ime_rect[2] = w;
    g_ime_rect[3] = h;
    pthread_mutex_unlock(&g_mu);
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

const char *salam_host_path(int kind)
{
    static char buf[1024];
    NSString *dir = nil;
    switch (kind) {
    case 0:
        dir = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES).firstObject;
        break;
    case 1:
        dir = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES).firstObject;
        break;
    case 2:
        dir = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
        break;
    case 3:
        dir = NSTemporaryDirectory();
        break;
    default:
        break;
    }
    buf[0] = 0;
    if (dir != nil) {
        [NSFileManager.defaultManager createDirectoryAtPath:dir withIntermediateDirectories:YES attributes:nil error:nil];
        while (dir.length > 1 && [dir hasSuffix:@"/"]) {
            dir = [dir substringToIndex:dir.length - 1];
        }
        strncpy(buf, dir.UTF8String, sizeof buf - 1);
        buf[sizeof buf - 1] = 0;
    }
    return buf;
}

static NSMutableDictionary *secure_query(const char *key)
{
    NSMutableDictionary *q = [NSMutableDictionary new];
    q[(__bridge id)kSecClass] = (__bridge id)kSecClassGenericPassword;
    q[(__bridge id)kSecAttrService] = @"salam.secure";
    q[(__bridge id)kSecAttrAccount] = [NSString stringWithUTF8String:key] ?: @"";
    return q;
}

int salam_host_secure_set(const char *key, const char *value)
{
    if (key == NULL || key[0] == 0 || value == NULL) {
        return 0;
    }
    NSMutableDictionary *q = secure_query(key);
    SecItemDelete((__bridge CFDictionaryRef)q);
    q[(__bridge id)kSecValueData] = [NSData dataWithBytes:value length:strlen(value)];
    q[(__bridge id)kSecAttrAccessible] = (__bridge id)kSecAttrAccessibleAfterFirstUnlockThisDeviceOnly;
    return SecItemAdd((__bridge CFDictionaryRef)q, NULL) == errSecSuccess ? 1 : 0;
}

const char *salam_host_secure_get(const char *key)
{
    static char copy[16384];
    copy[0] = 0;
    if (key == NULL) {
        return copy;
    }
    NSMutableDictionary *q = secure_query(key);
    q[(__bridge id)kSecReturnData] = @YES;
    q[(__bridge id)kSecMatchLimit] = (__bridge id)kSecMatchLimitOne;
    CFTypeRef out = NULL;
    if (SecItemCopyMatching((__bridge CFDictionaryRef)q, &out) == errSecSuccess && out != NULL) {
        NSData *d = (__bridge_transfer NSData *)out;
        size_t n = d.length < sizeof copy - 1 ? d.length : sizeof copy - 1;
        memcpy(copy, d.bytes, n);
        copy[n] = 0;
    }
    return copy;
}

int salam_host_secure_delete(const char *key)
{
    if (key == NULL) {
        return 0;
    }
    return SecItemDelete((__bridge CFDictionaryRef)secure_query(key)) == errSecSuccess ? 1 : 0;
}

static void on_main(void (^fn)(void))
{
    if ([NSThread isMainThread]) {
        fn();
    } else {
        dispatch_sync(dispatch_get_main_queue(), fn);
    }
}

const char *salam_host_info(const char *key)
{
    static char buf[2048];
    __block NSString *r = @"";
    if (key == NULL) {
        buf[0] = 0;
        return buf;
    }
    NSString *k = [NSString stringWithUTF8String:key];
    on_main(^{
        UIDevice *d = UIDevice.currentDevice;
        if ([k isEqualToString:@"model"]) {
            struct utsname u;
            uname(&u);
            const char *ident = getenv("SIMULATOR_MODEL_IDENTIFIER");
            r = [NSString stringWithUTF8String:ident != NULL ? ident : u.machine];
        } else if ([k isEqualToString:@"os"]) {
            r = @"iOS";
        } else if ([k isEqualToString:@"os_version"]) {
            r = d.systemVersion;
        } else if ([k isEqualToString:@"language"]) {
            r = NSLocale.preferredLanguages.firstObject ?: @"";
        } else if ([k isEqualToString:@"languages"]) {
            r = [NSLocale.preferredLanguages componentsJoinedByString:@","];
        } else if ([k isEqualToString:@"timezone"]) {
            r = NSTimeZone.localTimeZone.name;
        } else if ([k isEqualToString:@"density"]) {
            r = [NSString stringWithFormat:@"%g", UIScreen.mainScreen.scale];
        } else if ([k isEqualToString:@"app_version"]) {
            r = [NSBundle.mainBundle objectForInfoDictionaryKey:@"CFBundleShortVersionString"] ?: @"";
        } else if ([k isEqualToString:@"build"]) {
            r = [NSBundle.mainBundle objectForInfoDictionaryKey:@"CFBundleVersion"] ?: @"";
        } else if ([k isEqualToString:@"dark"]) {
            UITraitCollection *t = g_view != nil ? g_view.traitCollection : UIScreen.mainScreen.traitCollection;
            r = t.userInterfaceStyle == UIUserInterfaceStyleDark ? @"1" : @"0";
        } else if ([k isEqualToString:@"battery"] || [k isEqualToString:@"charging"]) {
            d.batteryMonitoringEnabled = YES;
            if ([k isEqualToString:@"battery"]) {
                r = d.batteryLevel < 0 ? @"-1" : [NSString stringWithFormat:@"%d", (int)(d.batteryLevel * 100 + 0.5)];
            } else {
                r = d.batteryState == UIDeviceBatteryStateCharging || d.batteryState == UIDeviceBatteryStateFull ? @"1" : @"0";
            }
        }
    });
    buf[0] = 0;
    strncpy(buf, r.UTF8String, sizeof buf - 1);
    buf[sizeof buf - 1] = 0;
    return buf;
}

static nw_path_monitor_t g_net_monitor;
static int g_net_state;

static int net_state_of(nw_path_t path)
{
    if (nw_path_get_status(path) != nw_path_status_satisfied) {
        return 0;
    }
    if (nw_path_uses_interface_type(path, nw_interface_type_wifi)) {
        return 1;
    }
    if (nw_path_uses_interface_type(path, nw_interface_type_cellular)) {
        return 2;
    }
    if (nw_path_uses_interface_type(path, nw_interface_type_wired)) {
        return 3;
    }
    return 4;
}

int salam_host_network(void)
{
    static dispatch_once_t once;
    static dispatch_semaphore_t first;
    dispatch_once(&once, ^{
        first = dispatch_semaphore_create(0);
        g_net_monitor = nw_path_monitor_create();
        nw_path_monitor_set_queue(g_net_monitor, dispatch_get_global_queue(QOS_CLASS_UTILITY, 0));
        __block int seen = 0;
        nw_path_monitor_set_update_handler(g_net_monitor, ^(nw_path_t path) {
            pthread_mutex_lock(&g_dev_mu);
            g_net_state = net_state_of(path);
            pthread_mutex_unlock(&g_dev_mu);
            if (!seen) {
                seen = 1;
                dispatch_semaphore_signal(first);
            } else {
                salam_host_post(SALAM_EV_DEVICE, 4, 0, 0, 0, 0, NULL);
            }
        });
        nw_path_monitor_start(g_net_monitor);
        dispatch_semaphore_wait(first, dispatch_time(DISPATCH_TIME_NOW, 500 * NSEC_PER_MSEC));
    });
    pthread_mutex_lock(&g_dev_mu);
    int s = g_net_state;
    pthread_mutex_unlock(&g_dev_mu);
    return s;
}

const char *salam_host_launch_url(void)
{
    static char copy[2048];
    pthread_mutex_lock(&g_url_mu);
    memcpy(copy, g_launch_url, sizeof copy);
    pthread_mutex_unlock(&g_url_mu);
    return copy;
}

const char *salam_host_take_url(void)
{
    static char copy[2048];
    pthread_mutex_lock(&g_url_mu);
    memcpy(copy, g_pending_url, sizeof copy);
    g_pending_url[0] = 0;
    pthread_mutex_unlock(&g_url_mu);
    return copy;
}

int salam_host_open_settings(void)
{
    dispatch_async(dispatch_get_main_queue(), ^{
        NSURL *u = [NSURL URLWithString:UIApplicationOpenSettingsURLString];
        [UIApplication.sharedApplication openURL:u options:@{} completionHandler:nil];
    });
    return 1;
}

int salam_host_screen(int what, int value)
{
    if (what == 5) {
        g_bright_asked = value;
    }
    if (what == 4) {
        if (g_bright_asked >= 0) {
            return g_bright_asked;
        }
        __block int pct = -1;
        on_main(^{
            pct = (int)(UIScreen.mainScreen.brightness * 100 + 0.5);
        });
        return pct;
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        UIViewController *c = g_controller;
        if (what == 0) {
            UIApplication.sharedApplication.idleTimerDisabled = value != 0;
        } else if (what == 1) {
            g_orient = value;
            if (@available(iOS 16.0, *)) {
                [c setNeedsUpdateOfSupportedInterfaceOrientations];
                UIWindowScene *ws = c.view.window.windowScene;
                if (ws != nil) {
                    UIInterfaceOrientationMask m = value == 1 ? UIInterfaceOrientationMaskPortrait : value == 2 ? UIInterfaceOrientationMaskLandscape : [c supportedInterfaceOrientations];
                    [ws requestGeometryUpdateWithPreferences:[[UIWindowSceneGeometryPreferencesIOS alloc] initWithInterfaceOrientations:m] errorHandler:^(NSError *e) {
                        (void)e;
                    }];
                }
            } else {
                [UIViewController attemptRotationToDeviceOrientation];
            }
        } else if (what == 2) {
            g_immersive = value;
            [c setNeedsStatusBarAppearanceUpdate];
            [c setNeedsUpdateOfHomeIndicatorAutoHidden];
        } else if (what == 3) {
            g_bar_style = value;
            [c setNeedsStatusBarAppearanceUpdate];
        } else if (what == 5) {
            if (value < 0) {
                if (g_saved_brightness >= 0) {
                    UIScreen.mainScreen.brightness = g_saved_brightness;
                    g_saved_brightness = -1;
                }
            } else {
                if (g_saved_brightness < 0) {
                    g_saved_brightness = UIScreen.mainScreen.brightness;
                }
                UIScreen.mainScreen.brightness = value / 100.0;
            }
        }
    });
    return 1;
}

static int g_auth_state;

int salam_host_auth_available(void)
{
    LAContext *c = [LAContext new];
    NSError *e = nil;
    return [c canEvaluatePolicy:LAPolicyDeviceOwnerAuthenticationWithBiometrics error:&e] ? 1 : 0;
}

int salam_host_auth_request(const char *reason)
{
    LAContext *c = [LAContext new];
    NSError *e = nil;
    if (![c canEvaluatePolicy:LAPolicyDeviceOwnerAuthenticationWithBiometrics error:&e]) {
        pthread_mutex_lock(&g_dev_mu);
        g_auth_state = 3;
        pthread_mutex_unlock(&g_dev_mu);
        return 0;
    }
    NSString *why = reason != NULL ? [NSString stringWithUTF8String:reason] : nil;
    if (why.length == 0) {
        why = @"Confirm it is you";
    }
    pthread_mutex_lock(&g_dev_mu);
    g_auth_state = 1;
    pthread_mutex_unlock(&g_dev_mu);
    [c evaluatePolicy:LAPolicyDeviceOwnerAuthenticationWithBiometrics localizedReason:why reply:^(BOOL ok, NSError *err) {
        pthread_mutex_lock(&g_dev_mu);
        g_auth_state = ok ? 2 : 3;
        pthread_mutex_unlock(&g_dev_mu);
        salam_host_post(SALAM_EV_DEVICE, 6, 0, 0, 0, 0, NULL);
    }];
    return 1;
}

int salam_host_auth_state(void)
{
    pthread_mutex_lock(&g_dev_mu);
    int s = g_auth_state;
    pthread_mutex_unlock(&g_dev_mu);
    return s;
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
