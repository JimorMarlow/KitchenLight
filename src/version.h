#pragma once
// version.h - KitchenLight version control

// NOTE: after version changing run script in terminal: 
// > python sync_version.py
// and config files: package.json will be patched...

#define KL_VERSION_MAJOR 0
#define KL_VERSION_MINOR 1
#define KL_VERSION_PATCH 5

// Хелпер для stringify
#define KL_STRINGIFY_HELPER(x) #x
#define KL_STRINGIFY(x) KL_STRINGIFY_HELPER(x)

//Собрать строку версии "X.X.X"
#define KL_VERSION_STRING KL_STRINGIFY(KL_VERSION_MAJOR) "." KL_STRINGIFY(KL_VERSION_MINOR) "." KL_STRINGIFY(KL_VERSION_PATCH)