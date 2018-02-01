/* Author: Benjmain Runco */

#include <stdio.h>

struct header {
  unsigned short start_of_file_marker;
  unsigned short APP1_marker;
  unsigned short APP1_length;
  char exif_string[4];
  unsigned short terminator;
  char endianness[2];
  unsigned short version;
  unsigned int offset;
};

struct TIFF {
  unsigned short tag_identifier;
  unsigned short data_type;
  unsigned int num_of_data_items;
  unsigned int offset;
};

/* prototype functions */
void get_manufacturer(FILE*, struct TIFF*);
void get_model(FILE*, struct TIFF*);
int get_picture_info(FILE*, struct TIFF*);

/* main function that takes arguments */
int main(int argc, char *argv[]) {

  /* declare structs */
  struct header hd;
  struct TIFF tiff;

  /* open binary file in read/write mode and return file pointer */
  FILE * fp = fopen(argv[1], "r+b");

  /* check if fp is NULL */
  if(fp != NULL) {

    /* read in header (first 20 bytes) */
    fread(&hd, sizeof(hd), 1, fp);

    /* check if file has an exif tag */
    if(hd.exif_string[0] != 'E' || hd.exif_string[1] != 'x' || hd.exif_string[2] != 'i' || hd.exif_string[3] != 'f') {
      printf("Exif Tag not found.\n");
      return 1;
    }

    /* check byte ordering */
    if(hd.endianness[0] == 'M' && hd.endianness[1] == 'M') {
      printf("Big Endian not supported.\n");
      return 1;
    }

    /* read 2-byte count (number of TIFF tags) */
    unsigned short count;
    fread(&count, sizeof(count), 1, fp);

    /* loop through count number of TIFF tags */
    int i;
    for(i = 0; i < count; i++) {

      /* read TIFF tag */
      fread(&tiff, sizeof(tiff), 1, fp);

      /* check for Manufacturer String and print if found */
      get_manufacturer(fp, &tiff);

      /* check for Camera Model String and print if found */
      get_model(fp, &tiff);

      /* check for detailed information about the picture and print if found */
      int found = get_picture_info(fp, &tiff);

      /* stop reading tags if info found */
      if (found) {
        break;
      }

    }
  }
  else {
    /* fp was NULL */
    printf("Could not open file.\n");
    return 1;
  }
  return 0;
}

void get_manufacturer(FILE *fp, struct TIFF *tiff) {

  /* keep track of place in file of next TIFF tag */
  int next_tag = ftell(fp);

  if(tiff->tag_identifier == 271) {
    fseek(fp, 12+tiff->offset, SEEK_SET);

    /* read in manufacturer string into array of chars */
    char manufacturer_string[tiff->num_of_data_items];
    fread(&manufacturer_string, sizeof(manufacturer_string), 1, fp);

    printf("Manufacturer: \t%s\n", manufacturer_string);

    /* move file pointer back to position of next TIFF tag*/
    fseek(fp, next_tag, SEEK_SET);
  }

}

void get_model(FILE *fp, struct TIFF *tiff) {

  /* keep track of place in file of next TIFF tag */
  int next_tag = ftell(fp);

  if(tiff->tag_identifier == 272) {
    /* move file pointer to beginning of model string */
    fseek(fp, 12+tiff->offset, SEEK_SET);

    /* read in model string into array of chars */
    char model_string[tiff->num_of_data_items];
    fread(&model_string, sizeof(model_string), 1, fp);

    printf("Model: \t\t%s\n", model_string);

    /* move file pointer back to position of next TIFF tag*/
    fseek(fp, next_tag, SEEK_SET);
  }

}

int get_picture_info(FILE *fp, struct TIFF *tiff) {

  /* keep track of place in file of next TIFF tag */
  int next_tag = ftell(fp);

  /* if ID matches Exif sub block address tag ID */
  if(tiff->tag_identifier == 0x8769) {
    fseek(fp, 12+tiff->offset, SEEK_SET);
    unsigned short count;
    fread(&count, sizeof(count), 1, fp);

    /* declare new struct to hold image info */
    struct TIFF info_tiff;

    /* loop through count number of tiff tags */
    int j;
    for(j = 0; j < count; j++) {
      fread(&info_tiff, sizeof(info_tiff), 1, fp);

      int next_tag = ftell(fp);

      /* if ID matches Width in pixels tag ID */
      if(info_tiff.tag_identifier == 0xA002)
        printf("Width: \t\t%d pixels\n", info_tiff.offset);

      /* if ID matches Height in pixels tag ID*/
      if(info_tiff.tag_identifier == 0xA003)
        printf("Height: \t%d pixels\n", info_tiff.offset);

      /* if ID matches ISO speed tag ID*/
      if(info_tiff.tag_identifier == 0x8827) {
        printf("ISO: \t\tISO %d\n", info_tiff.offset);
      }

      /* if ID matches Date taken tag ID*/
      if (info_tiff.tag_identifier == 0x9003) {
        fseek(fp, 12+info_tiff.offset, SEEK_SET);
        char date_taken[info_tiff.num_of_data_items];
        fread(&date_taken, sizeof(date_taken), 1, fp);
        printf("Date Taken: \t%s\n", date_taken);
        fseek(fp, next_tag, SEEK_SET);
      }

      /* if ID matches Exposure speed tag ID*/
      if(info_tiff.tag_identifier == 0x829a) {
        fseek(fp, 12+info_tiff.offset, SEEK_SET);
        unsigned int exposure1;
        unsigned int exposure2;
        fread(&exposure1, sizeof(exposure1), 1, fp);
        fread(&exposure2, sizeof(exposure2), 1, fp);
        printf("Exposure Time: \t%d/%d second\n", exposure1, exposure2);
        fseek(fp, next_tag, SEEK_SET);
      }

      /* if ID matches F-stop tag ID*/
      if(info_tiff.tag_identifier == 0x829d) {
        fseek(fp, 12+info_tiff.offset, SEEK_SET);
        unsigned int fstop1;
        unsigned int fstop2;
        fread(&fstop1, sizeof(fstop1), 1, fp);
        fread(&fstop2, sizeof(fstop2), 1, fp);
        double fstop = (double) fstop1/fstop2;
        printf("F-stop: \tf/%.1f \n", fstop);
        fseek(fp, next_tag, SEEK_SET);
      }

      /* if ID matches Lens focal length tag ID*/
      if(info_tiff.tag_identifier == 0x920A) {
        fseek(fp, 12+info_tiff.offset, SEEK_SET);
        unsigned int focal_length1;
        unsigned int focal_length2;
        fread(&focal_length1, sizeof(focal_length1), 1, fp);
        fread(&focal_length2, sizeof(focal_length2), 1, fp);
        printf("Focal Length: \t%d mm \n", focal_length1/focal_length2);
        fseek(fp, next_tag, SEEK_SET);
      }

    }
    return 1;
  }
  return 0;
}
