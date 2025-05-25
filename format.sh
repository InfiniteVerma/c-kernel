git diff --name-only --cached -- '*.cpp' '*.h' | xargs clang-format -i
