s/build\/\([a-zA-Z]*\)\/Debug/build\/\1\/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/g
s/build\/\([a-zA-Z]*\)\/Release/build\/\1\/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/g
s/build\/\([a-zA-Z]*\)\/RelWithDebInfo/build\/\1\/Release$(EFFECTIVE_PLATFORM_NAME)/g
s/build\/\([a-zA-Z]*\)\/MinSizeRel/build\/\1\/Release$(EFFECTIVE_PLATFORM_NAME)/g
