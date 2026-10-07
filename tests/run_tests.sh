#!/usr/bin/env bash
set -e

echo "=================================================="
echo "Starting comprehensive test suite for simplified ls"
echo "=================================================="

TEST_DIR=$(mktemp -d -t ls_test_XXXXXX)
trap 'rm -rf "$TEST_DIR"' EXIT

echo "Using temporary test directory: $TEST_DIR"

# 1. Setup test fixtures
mkdir -p "$TEST_DIR/empty_dir"
mkdir -p "$TEST_DIR/dir1/subdir1"
mkdir -p "$TEST_DIR/dir2"

touch "$TEST_DIR/file1.txt"
echo "Hello World" > "$TEST_DIR/file2.txt"
echo "Longer content for size test" > "$TEST_DIR/file3_long.txt"
touch "$TEST_DIR/.hidden1"
touch "$TEST_DIR/.hidden2"

# Symlinks
ln -s "$TEST_DIR/file1.txt" "$TEST_DIR/symlink_file"
ln -s "$TEST_DIR/dir1" "$TEST_DIR/symlink_dir"
ln -s "$TEST_DIR/nonexistent_target" "$TEST_DIR/broken_symlink"

# File with spaces and special characters
touch "$TEST_DIR/file with spaces.txt"
# Non-printable character in filename (ASCII 1)
touch "$TEST_DIR/file"$'\001'"nonprint.txt"

# FIFO
mkfifo "$TEST_DIR/test_fifo" || true

echo "Fixtures created successfully."

# 2. Test basic execution
echo "--- Test 1: Basic listing ---"
./ls "$TEST_DIR" > /dev/null

echo "--- Test 2: -a and -A options ---"
A_OUT=$(./ls -a "$TEST_DIR")
echo "$A_OUT" | grep -q "^\.$"
echo "$A_OUT" | grep -q "^\.\.$"
echo "$A_OUT" | grep -q "^\.hidden1$"

AA_OUT=$(./ls -A "$TEST_DIR")
if echo "$AA_OUT" | grep -q "^\.$"; then
    echo "ERROR: -A should not list '.'"
    exit 1
fi
echo "$AA_OUT" | grep -q "^\.hidden1$"

echo "--- Test 3: -l and -n long listing ---"
L_OUT=$(./ls -l "$TEST_DIR")
echo "$L_OUT" | grep -q "^total "
echo "$L_OUT" | grep -q "symlink_file -> "

N_OUT=$(./ls -n "$TEST_DIR")
echo "$N_OUT" | grep -q " $(id -u) "

echo "--- Test 4: -h human-readable ---"
H_OUT=$(./ls -lh "$TEST_DIR")
echo "$H_OUT" | grep -q "total "

echo "--- Test 5: -s and -k block counts ---"
S_OUT=$(./ls -s "$TEST_DIR")
SK_OUT=$(./ls -sk "$TEST_DIR")

echo "--- Test 6: -i inode numbers ---"
I_OUT=$(./ls -i "$TEST_DIR")

echo "--- Test 7: -F classification indicators ---"
F_OUT=$(./ls -F "$TEST_DIR")
echo "$F_OUT" | grep -q "dir1/"
echo "$F_OUT" | grep -q "symlink_file@"
if [ -p "$TEST_DIR/test_fifo" ]; then
    echo "$F_OUT" | grep -q "test_fifo|"
fi

echo "--- Test 8: Sorting (-S, -t, -r, -f) ---"
./ls -S "$TEST_DIR" > /dev/null
./ls -Sr "$TEST_DIR" > /dev/null
./ls -t "$TEST_DIR" > /dev/null
./ls -tr "$TEST_DIR" > /dev/null
./ls -f "$TEST_DIR" > /dev/null

echo "--- Test 9: -d directory as file ---"
D_OUT=$(./ls -d "$TEST_DIR/dir1")
[ "$D_OUT" = "$TEST_DIR/dir1" ]

echo "--- Test 10: -R recursive listing ---"
R_OUT=$(./ls -R "$TEST_DIR")
echo "$R_OUT" | grep -q "dir1/subdir1:"

echo "--- Test 11: Non-printable characters (-q and -w) ---"
Q_OUT=$(./ls -q "$TEST_DIR")
echo "$Q_OUT" | grep -q "file?nonprint.txt"

W_OUT=$(./ls -w "$TEST_DIR")
echo "$W_OUT" | grep -q "file"$'\001'"nonprint.txt"

echo "--- Test 12: Empty directory ---"
./ls "$TEST_DIR/empty_dir"
EMPTY_L=$(./ls -l "$TEST_DIR/empty_dir")
echo "$EMPTY_L" | grep -q "^total 0"

echo "--- Test 13: Multiple operands with mixed files and dirs ---"
MULT_OUT=$(./ls "$TEST_DIR/file1.txt" "$TEST_DIR/dir1" "$TEST_DIR/dir2")
echo "$MULT_OUT" | grep -q "^$TEST_DIR/file1.txt"
echo "$MULT_OUT" | grep -q "^$TEST_DIR/dir1:"
echo "$MULT_OUT" | grep -q "^$TEST_DIR/dir2:"

echo "--- Test 14: Special character/block devices ---"
if [ -c /dev/null ]; then
    ./ls -l /dev/null | grep -q "^c"
fi

echo "--- Test 15: Error handling and exit status ---"
set +e
./ls "$TEST_DIR/nonexistent_file" 2>/dev/null
ERR_STATUS=$?
set -e
if [ $ERR_STATUS -eq 0 ]; then
    echo "ERROR: ls should return non-zero for nonexistent file"
    exit 1
fi

echo "=================================================="
echo "All tests passed successfully!"
echo "=================================================="
