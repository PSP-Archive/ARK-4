# Make an automatic library - will be static or dynamic based on user setting
add_library(lz4 lz4/lib/lz4.c lz4/lib/lz4hc.c)

# We need this directory, and users of our library will need it too
target_include_directories(lz4 PUBLIC lz4/lib/)