
const index_t INDEX_Z;
const uint32_t INDEX_SIZE = sizeof(INDEX_Z);

volatile uint32_t base_idx = 0;

void init_index (){
    File f;

    Serial.println("Initializing index file");
    if(!SD.exists(INDEX_FILE)){
        Serial.println("Creating default index file");
        File f = SD.open(INDEX_FILE, "w");
        index_t item;
        for(int i = 0; i < INDEX_ENTRIES; i++){
            item = INDEX_Z;
            item.idx = i;
            item.base = (i == base_idx) ? true : false;
            f.seek(i*INDEX_SIZE);
            f.write((uint8_t *)&item, INDEX_SIZE);
        }
        f.close();
    }

    index_t item;

    Serial.println("DEBUG: printing all indexes:");
    for(int i = 0; i < INDEX_ENTRIES; i++){
        read_index(&item, i);
        print_index(&item);
    }

    Serial.println("Finding base index");
    for(int i = 0; i < INDEX_ENTRIES; i++){
        if((read_index(&item, i) >= 0) && item.valid && item.base){
            base_idx = item.idx;
            break;
        }
    }
    Serial.println("Got base index: " + String(base_idx));

    Serial.println("Removing files from the index if they don't exist on the SD card");
    for(int i = 0; i < INDEX_ENTRIES; i++){
        if(read_index(&item, i) >= 0 && item.valid && !SD.exists("/" + String((const char *)item.name))){
            item.valid = false;
            write_index(&item, i);
        }
    }

    Serial.println("Adding images to the index that are present on the SD card but missing from the index");
    File root = SD.open("/");
    if (root) {
        index_t dummy;
        root.rewindDirectory();
        File file = root.openNextFile();
        while(file){
            String name = String(file.name());
            Serial.println("Checking on SD card file: " + name);
            if(name.endsWith(".jpg") || name.endsWith(".gif") || name.endsWith(".txt") || name.endsWith(".bmp")){
                if(find_index(file.name(), &dummy) == -1){
                    add_index(file.name(), file.size());
                }
            }
            file = root.openNextFile();
        }
        root.close();
    } else {
        Serial.println("Error, couldn't open root directory");
    }

    Serial.println("Done initializing index file");
}

int write_index (index_t *item, uint32_t idx){
    // append is not what you want here, reference cplusplus.com
    File f = SD.open(INDEX_FILE, "r+");
    if(f && idx < INDEX_ENTRIES){
        item->idx = idx;
        f.seek(idx * INDEX_SIZE);
        f.write((uint8_t *)item, INDEX_SIZE);
        f.close();
        //Serial.println("Wrote to index: " + String(idx));
        return 0;
    } else {
        Serial.println("Error writing to index: " + String(idx));
        return -1;
    }
}

int read_index(index_t *item, uint32_t idx){
    File f = SD.open(INDEX_FILE, "r");
    if(f && idx < INDEX_ENTRIES){
        f.seek(idx * INDEX_SIZE);
        f.read((uint8_t *)item, INDEX_SIZE);
        f.close();
        //Serial.println("Read from index: " + String(idx));
        return 0;
    } else {
        Serial.println("Error reading from index: " + String(idx));
        return -1;
    }
}

int find_index (const char *name, index_t *item) {
    File f = SD.open(INDEX_FILE, "r");
    if(f){
        for(int idx = 0; idx < INDEX_ENTRIES; idx++){
            f.seek(idx * INDEX_SIZE);
            f.read((uint8_t *)item, INDEX_SIZE);
            if(item->valid && !strcmp(name, (const char *)item->name)){
                f.close();
                return idx;
            }
        }
        f.close();
        return -1;
    } else {
        Serial.println("Error finding index name: " + String(name));
        return -1;
    }
}

int add_index (const char *name, uint32_t size){

    uint32_t this_idx = base_idx;
    index_t item;

    Serial.println("Attempting to add index: " + String(name) + ", size: " + String(size));

    for(int i = 0; i < INDEX_ENTRIES; i++){
        read_index(&item, this_idx);
        if(!item.valid) break;
        this_idx = (this_idx + 1) & (INDEX_ENTRIES-1); 
        if(i == INDEX_ENTRIES-1){
            Serial.println("Error finding free space for new addition");
            return -1;
        }
    }

    /*
        Point the base backward to the new item
        Point the last item forward to the new item
        Point the new item forward to the base and backward to the last item
    */

    int flag = 0;

    read_index(&item, base_idx);
    uint32_t old_prev = item.prev;
    item.prev = this_idx;
    write_index(&item, base_idx);
    print_index(&item);

    read_index(&item, old_prev);
    item.next = this_idx;
    write_index(&item, old_prev);
    print_index(&item);

    item.valid = true;
    item.base = (this_idx == base_idx) ? true : false;
    item.size = size;
    item.prev = old_prev;
    item.next = base_idx;
    strcpy((char *)item.name, name);
    write_index(&item, this_idx);
    print_index(&item);

    return 0;
}


int delete_index (const char *name){
    Serial.println("Attempting to delete index: " + String(name));

    index_t item;
    uint32_t prev, next;

    int this_idx = find_index(name, &item);
    if(this_idx < 0){
        Serial.println("Error attempting to delete index as it's already missing");
        return -1;
    }

    /*
        Connect the next and prev indexes of the deleted item
    */

    next = item.next;
    prev = item.prev;

    memcpy(&item, &INDEX_Z, INDEX_SIZE);
    write_index(&item, this_idx);

    read_index(&item, prev);
    item.next = next;
    write_index(&item, prev);

    read_index(&item, next);
    item.prev = prev;
    if(this_idx == base_idx){
        base_idx = next;
        item.base = 1;
    }
    write_index(&item, next);

    return 0;
}

int traverse_index (index_t *item, bool restart, bool forward){
    uint32_t fut;
    fut = restart ? base_idx : forward ? item->next : item->prev;
    //Serial.println("Traversing to: " + String(fut));
    if(read_index(item, fut) < 0){
        Serial.println("Error reading next item while traversing index: " + String(fut));
        return -1;
    }
    if(!item->valid){
        return -1;
    } else if ((forward ? item->next : item->prev) == base_idx){
        return 1;
    } else {
        return 0;
    }
}

void print_index (index_t *item){
    Serial.printf("  valid = %d, base = %d, idx = %d, prev = %d, next = %d, name=%s, size %d\n\r", item->valid, item->base, item->idx, item->prev, item->next, String((const char *)item->name), item->size);
}
