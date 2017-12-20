#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define BYTE unsigned char

struct task_data {
  int begin;
  int end;
};

struct file_data {
  BYTE* pixel;
  unsigned long length;
};

// global
int num_of_cpu;
int total_task;
int task_per_thread;
//BYTE block[16];
BYTE* block;
BYTE key[16 * (14 + 1)];
BYTE sbox[] = {
  99,124,119,123,242,107,111,197,48,1,103,43,254,215,171,
  118,202,130,201,125,250,89,71,240,173,212,162,175,156,164,114,192,183,253,
  147,38,54,63,247,204,52,165,229,241,113,216,49,21,4,199,35,195,24,150,5,154,
  7,18,128,226,235,39,178,117,9,131,44,26,27,110,90,160,82,59,214,179,41,227,
  47,132,83,209,0,237,32,252,177,91,106,203,190,57,74,76,88,207,208,239,170,
  251,67,77,51,133,69,249,2,127,80,60,159,168,81,163,64,143,146,157,56,245,
  188,182,218,33,16,255,243,210,205,12,19,236,95,151,68,23,196,167,126,61,
  100,93,25,115,96,129,79,220,34,42,144,136,70,238,184,20,222,94,11,219,224,
  50,58,10,73,6,36,92,194,211,172,98,145,149,228,121,231,200,55,109,141,213,
  78,169,108,86,244,234,101,122,174,8,186,120,37,46,28,166,180,198,232,221,
  116,31,75,189,139,138,112,62,181,102,72,3,246,14,97,53,87,185,134,193,29,
  158,225,248,152,17,105,217,142,148,155,30,135,233,206,85,40,223,140,161,
  137,13,191,230,66,104,65,153,45,15,176,84,187,22
};
BYTE shift_row_tab[] = { 0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12, 1, 6, 11 };
BYTE sbox_inverse[256];
BYTE shift_row_tab_inverse[16];
BYTE xtime[256];

void print_bytes(BYTE b[], int len) {
  for (int i = 0; i < len; i++)
    printf("%d ", b[i]);
  printf("\n");
}

void sub_bytes(BYTE state[], BYTE sbox[]) {
  int i;
  for (i = 0; i < 16; i++)
    state[i] = sbox[state[i]];
}

void add_round_key(BYTE state[], BYTE rkey[]) {
  int i;
  for (i = 0; i < 16; i++)
    state[i] ^= rkey[i];
}

void shift_rows(BYTE state[], BYTE shifttab[]) {
  BYTE h[16];
  memcpy(h, state, 16);
  for (int i = 0; i < 16; i++)
    state[i] = h[shifttab[i]];
}

void mix_columns(BYTE state[]) {
  for (int i = 0; i < 16; i += 4) {
    BYTE s0 = state[i + 0], s1 = state[i + 1];
    BYTE s2 = state[i + 2], s3 = state[i + 3];
    BYTE h = s0 ^ s1 ^ s2 ^ s3;
    state[i + 0] ^= h ^ xtime[s0 ^ s1];
    state[i + 1] ^= h ^ xtime[s1 ^ s2];
    state[i + 2] ^= h ^ xtime[s2 ^ s3];
    state[i + 3] ^= h ^ xtime[s3 ^ s0];
  }
}

void inverse_mix_columns(BYTE state[]) {
  for (int i = 0; i < 16; i += 4) {
    BYTE s0 = state[i + 0], s1 = state[i + 1];
    BYTE s2 = state[i + 2], s3 = state[i + 3];
    BYTE h = s0 ^ s1 ^ s2 ^ s3;
    BYTE xh = xtime[h];
    BYTE h1 = xtime[xtime[xh ^ s0 ^ s2]] ^ h;
    BYTE h2 = xtime[xtime[xh ^ s1 ^ s3]] ^ h;
    state[i + 0] ^= h1 ^ xtime[s0 ^ s1];
    state[i + 1] ^= h2 ^ xtime[s1 ^ s2];
    state[i + 2] ^= h1 ^ xtime[s2 ^ s3];
    state[i + 3] ^= h2 ^ xtime[s3 ^ s0];
  }
}

void init() {
  int i;
  for (i = 0; i < 256; i++)
    sbox_inverse[sbox[i]] = i;

  for (i = 0; i < 16; i++)
    shift_row_tab_inverse[shift_row_tab[i]] = i;

  for (i = 0; i < 128; i++) {
    xtime[i] = i << 1;
    xtime[128 + i] = (i << 1) ^ 0x1b;
  }
}

void release() {}

int expend_key(BYTE key[], int key_len) {
  int kl = key_len, ks, Rcon = 1, i, j;
  BYTE temp[4], temp2[4];
  switch (kl) {
    case 16: ks = 16 * (10 + 1); break;
    case 24: ks = 16 * (12 + 1); break;
    case 32: ks = 16 * (14 + 1); break;
    default:
      printf("expend_key: Only key lengths of 16, 24 or 32 bytes allowed!");
  }
  for (i = kl; i < ks; i += 4) {
    memcpy(temp, &key[i-4], 4);
    if (i % kl == 0) {
      temp2[0] = sbox[temp[1]] ^ Rcon;
      temp2[1] = sbox[temp[2]];
      temp2[2] = sbox[temp[3]];
      temp2[3] = sbox[temp[0]];
      memcpy(temp, temp2, 4);
      if ((Rcon <<= 1) >= 256)
        Rcon ^= 0x11b;
    }
    else if ((kl > 24) && (i % kl == 16)) {
      temp2[0] = sbox[temp[0]];
      temp2[1] = sbox[temp[1]];
      temp2[2] = sbox[temp[2]];
      temp2[3] = sbox[temp[3]];
      memcpy(temp, temp2, 4);
    }
    for (j = 0; j < 4; j++)
      key[i + j] = key[i + j - kl] ^ temp[j];
  }
  return ks;
}

void _encrypt(void* task) {
  // var
  int begin = ((struct task_data*) task)->begin;
  int end = ((struct task_data*) task)->end;

  for (int i = begin; i <= end; i++) {
    sub_bytes(block, sbox);
    shift_rows(block, shift_row_tab);
    mix_columns(block);
    add_round_key(block, &key[i * 16]);
  }
}

void encrypt(int key_len) {
  // var
  int i;
  int l = key_len;
  struct task_data* task = malloc(num_of_cpu * sizeof(struct task_data));

  print_bytes(block, 16);
  add_round_key(block, &key[0]);

  // var
  pthread_attr_t attr;
  pthread_t* threads = malloc(num_of_cpu * sizeof(pthread_t));

  pthread_attr_init(&attr);

  for (int j = 0; j < num_of_cpu; j++) {
    // var
    task[j].begin = j * task_per_thread + 1;
    task[j].end = (j + 1) * task_per_thread;
    task[j].end = task[j].end > total_task ? total_task : task[j].end;

    if (pthread_create(&threads[i], &attr, _encrypt, (void *) &task[j])) {
      fprintf(stderr, "%s: line %d\n", __func__, __LINE__);
      exit(1);
    }
  }

  for (int j = 0; j < num_of_cpu; j++) {
    pthread_join(threads[j], NULL);
  }

  sub_bytes(block, sbox);
  shift_rows(block, shift_row_tab);
  add_round_key(block, &key[l - 16]);
}

void _decrypt(void* task) {
  // var
  int begin = ((struct task_data*) task)->begin;
  int end = ((struct task_data*) task)->end;

  for (int i = begin; i <= end; i++) {
    add_round_key(block, &key[i * 16]);
    inverse_mix_columns(block);
    shift_rows(block, shift_row_tab_inverse);
    sub_bytes(block, sbox_inverse);
  }
}

void decrypt(int key_len) {
  // var
  int i;
  int l = key_len;
  struct task_data* task = malloc(num_of_cpu * sizeof(struct task_data));

  add_round_key(block, &key[l - 16]);
  shift_rows(block, shift_row_tab_inverse);
  sub_bytes(block, sbox_inverse);

  // var
  pthread_attr_t attr;
  pthread_t* threads = malloc(num_of_cpu * sizeof(pthread_t));

  pthread_attr_init(&attr);

  for (int j = 0; j < num_of_cpu; j++) {
    // var
    task[j].begin = j * task_per_thread + 1;
    task[j].end = (j + 1) * task_per_thread;
    task[j].end = task[j].end > total_task ? total_task : task[j].end;

    if (pthread_create(&threads[i], &attr, _decrypt, (void *) &task[j])) {
      fprintf(stderr, "%s: line %d\n", __func__, __LINE__);
      exit(1);
    }
  }

  for (int j = 0; j < num_of_cpu; j++) {
    pthread_join(threads[j], NULL);
  }

  add_round_key(block, &key[0]);
}

struct file_data* read_file(char *file_name) {
  // var
  FILE *fd;
  struct file_data* file = malloc(sizeof(struct file_data));

  // load file
  fd = fopen(file_name, "rb");
  if(!fd){
    fprintf(stderr, "%s: line %d\n", __func__, __LINE__);
    return 0;
  }

  // get file length
  fseek(fd, 0, SEEK_END);
  file->length = ftell(fd);
  fseek(fd, 0, SEEK_SET);

  file->pixel = (BYTE*) malloc(file->length + 1);
  if(!file->pixel){
    fprintf(stderr, "%s: line %d\n", __func__, __LINE__);
    fclose(fd);
    return 0;
  }

  fread(file->pixel, file->length, sizeof(BYTE), fd);
  fclose(fd);

  return file;
}

void write_file(struct file_data* file) {
  // var
  FILE* fd;

  fd = fopen("out.jpg", "wb");
  if(!fd){
    fprintf(stderr, "%s: line %d\n", __func__, __LINE__);
  }

  fwrite(file->pixel, 1, file->length * sizeof(BYTE), fd);
  fclose(fd);
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("Usage: ./aes_pthread num_of_cpu\n");
    exit(1);
  }

  // var
  int key_len = 32;
  int max_key_len = 16 * (14 + 1);
  int block_len = 16;
  int expend_key_len;
  struct file_data* file;

  // init
  init();
  num_of_cpu = atoi(argv[1]);

  file = read_file("minions.jpg");

  //write_file(file);

  printf("file.length = %d\n", file->length);
  printf("file.size = %d\n", (file->length - 13) * sizeof(BYTE));

  int new_file_length = file->length - 13;

  if (new_file_length % 4 != 0) {
    new_file_length = (new_file_length / 4 + 1) * 4;
  }

  block = malloc(new_file_length * sizeof(BYTE));

  memcpy(&block, file->pixel[11], new_file_length);

  /*
  for (int i = 0; i < 16; i++)
    block[i] = 0x11 * i;
  */

  block_len = new_file_length;

  printf("Original message:\n");
  //print_bytes(block, 16);

  for (int i = 0; i < key_len; i++)
    key[i] = i;

  printf("\nOriginal key:\n");
  print_bytes(key, key_len);

  expend_key_len = expend_key(key, key_len);

  // init var for pthread
  total_task = key_len / 16 - 2;
  task_per_thread = total_task / num_of_cpu + 1;

  printf("\nExpended key:\n");
  print_bytes(key, expend_key_len);

  encrypt(expend_key_len);

  printf("\nEncrypted:\n");
  print_bytes(block, block_len);

  decrypt(expend_key_len);

  printf("\nDecrypted:\n");
  print_bytes(block, block_len);

  release();
}
