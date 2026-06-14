bool compare_and_swap(int* ptr, int old_value, int new_value) {
    if (*ptr == old_value) {
        *ptr = new_value;
        return true;
    }
    return false;
}