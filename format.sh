find . -regex '.*\.\(c\|h\|cpp\|hpp\)' -exec clang-format -i {} +
echo "Formatting done"
