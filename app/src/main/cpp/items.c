#include <jni.h>
#include <android/log.h>
#include <link.h>
#include <pthread.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
typedef struct { void *klass, *monitor; int32_t length; uint16_t chars[]; } Il2CppString;
static int (*original_count)(Il2CppString *, void *);
static int (*original_use)(Il2CppString *, int, Il2CppString *, void *);
static int is_item(const Il2CppString *id) {
    static const char *names[] = {"hammer", "doubleball", "thunder", "booster", "laser", "goldenaim", "bounceshield", "undo", "bubbleball", "flameball", "trioball", "removeghost", "add5ball", "powerball"};
    if (!id || id->length < 1 || id->length > 32) return 0;
    for (size_t n = 0; n < sizeof(names) / sizeof(names[0]); n++) {
        size_t len = strlen(names[n]);
        if ((size_t)id->length != len) continue;
        size_t i = 0;
        while (i < len && id->chars[i] == (unsigned char)names[n][i]) i++;
        if (i == len) return 1;
    }
    return 0;
}
static int count_hook(Il2CppString *id, void *method) {
    return is_item(id) ? 10000 : original_count(id, method);
}
static int use_hook(Il2CppString *id, int count, Il2CppString *where, void *method) {
    return is_item(id) ? 1 : original_use(id, count, where, method);
}
static void jump(void *at, void *target) {
    const uint32_t code[] = {0x58000050, 0xd61f0200};
    memcpy(at, code, sizeof(code));
    memcpy((char *)at + 8, &target, sizeof(target));
    __builtin___clear_cache(at, (char *)at + 16);
}
static int hook(void *target, void *replacement, void **original) {
    void *trampoline = mmap(NULL, 32, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (trampoline == MAP_FAILED) return 0;
    memcpy(trampoline, target, 16);
    jump((char *)trampoline + 16, (char *)target + 16);
    long page = sysconf(_SC_PAGESIZE);
    uintptr_t start = (uintptr_t)target & ~(uintptr_t)(page - 1);
    size_t size = ((uintptr_t)target + 16 - start + page - 1) & ~(size_t)(page - 1);
    if (mprotect((void *)start, size, PROT_READ | PROT_WRITE | PROT_EXEC)) { munmap(trampoline, 32); return 0; }
    *original = trampoline;
    jump(target, replacement);
    mprotect((void *)start, size, PROT_READ | PROT_EXEC);
    return 1;
}
static uintptr_t il2cpp_base;
typedef struct { void *source; void *result; int16_t token; } PurchaseTask;
_Static_assert(sizeof(PurchaseTask) == 24 && offsetof(PurchaseTask, result) == 8, "UniTask layout");
static PurchaseTask purchase_hook(Il2CppString *product, void *method) {
    (void)product; (void)method;
    void *(*domain_get)(void) = (void *)(il2cpp_base + 0x3290018);
    const void **(*assemblies_get)(void *, size_t *) = (void *)(il2cpp_base + 0x3290024);
    void *(*image_get)(const void *) = (void *)(il2cpp_base + 0x328fb1c);
    void *(*class_from_name)(void *, const char *, const char *) = (void *)(il2cpp_base + 0x328fb54);
    void *(*object_new)(void *) = (void *)(il2cpp_base + 0x32903f4);
    size_t count = 0;
    const void **assemblies = assemblies_get(domain_get(), &count);
    void *klass = NULL;
    for (size_t i = 0; assemblies && i < count && !klass; i++)
        klass = class_from_name(image_get(assemblies[i]), "P1SPlatform.Store", "PurchaseOutput");
    void *output = klass ? object_new(klass) : NULL;
    if (output) *((uint8_t *)output + 0x10) = 1;
    else __android_log_print(ANDROID_LOG_ERROR, "BrickOutItems", "Cannot create purchase result");
    PurchaseTask result = {NULL, output, 0};
    return result;
}
static int find_library(struct dl_phdr_info *info, size_t size, void *data) {
    (void)size;
    if (strstr(info->dlpi_name, "/libil2cpp.so")) { *(uintptr_t *)data = info->dlpi_addr; return 1; }
    return 0;
}
static void *worker(void *unused) {
    (void)unused;
    uintptr_t base = 0;
    while (!base) { dl_iterate_phdr(find_library, &base); if (!base) usleep(10000); }
    static const unsigned char count_bytes[16] = {0xff,0x43,0x01,0xd1,0xe8,0x13,0x00,0xfd,0xfe,0x57,0x03,0xa9,0xf4,0x4f,0x04,0xa9};
    static const unsigned char use_bytes[16] = {0xff,0x83,0x01,0xd1,0xe8,0x13,0x00,0xfd,0xfe,0x5f,0x03,0xa9,0xf6,0x57,0x04,0xa9};
    void *count = (void *)(base + 0x38f3e60), *use = (void *)(base + 0x38f4078);
    if (memcmp(count, count_bytes, 16) || memcmp(use, use_bytes, 16)) {
        __android_log_print(ANDROID_LOG_ERROR, "BrickOutItems", "Unsupported library version");
        return NULL;
    }
    static const unsigned char purchase_bytes[16] = {0xff,0x43,0x02,0xd1,0xe8,0x33,0x00,0xfd,0xfe,0x37,0x00,0xf9,0xf6,0x57,0x07,0xa9};
    void *purchase = (void *)(base + 0x38ec5f8), *original_purchase = NULL;
    il2cpp_base = base;
    int p = !memcmp(purchase, purchase_bytes, 16) && hook(purchase, purchase_hook, &original_purchase);
    __android_log_print(ANDROID_LOG_INFO, "BrickOutItems", "purchase=%d", p);
    int a = hook(count, count_hook, (void **)&original_count);
    int b = hook(use, use_hook, (void **)&original_use);
    __android_log_print(ANDROID_LOG_INFO, "BrickOutItems", "count=%d use=%d", a, b);
    return NULL;
}
JNIEXPORT void JNICALL Java_dev_brickout_items_Entry_start(JNIEnv *env, jclass cls) {
    (void)env; (void)cls;
    pthread_t thread;
    if (!pthread_create(&thread, NULL, worker, NULL)) pthread_detach(thread);
}
