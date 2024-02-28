using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";
typedef struct {
  const string *key;
  void *base;
} keyStruct;

imdb::imdb(const string& directory)
{
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;
  
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const
{
  return !( (actorInfo.fd == -1) || 
	    (movieInfo.fd == -1) ); 
}

// you should be implementing these two methods right here...
// static void sanity_check (char *ptr) {
//   while ((*ptr) != '\0') {
//     printf("%c", *ptr);
//     ptr++;
//   }
//   ptr++;
//   short n = *(short *)ptr;
//   cout << n << endl;
// } 
static int bsearchCompare (const void* a, const void* b) {
  keyStruct *param = (keyStruct *) a;
  int offset = *(int *) b;

  const char *A = *(const char **) param->key;
  const char *B = ((char *) param->base + offset);
  // printf("%s, %s\n", A, B);
  return strcmp(A, B);
}
bool imdb::getCredits(const string& player, vector<film>& films) const {
  int *start = (int *) actorInfo.fileMap;
  char *movieStart = (char *)movieInfo.fileMap;
  keyStruct *param = new keyStruct;
  param->key = &player;
  param->base = start;

  int *actorOffset = (int *) bsearch (param, (void *) (start+1), *start, sizeof(int), bsearchCompare);
  if (actorOffset == NULL) {
    return false;
  }

  char *actorStart = (char *)start + *actorOffset;
  char *loc = (char *) (actorStart + strlen(actorStart));
  // Skip \0
  while (*loc == '\0') {
    loc++;
  }
  short nFilms = *(short *)loc;
  loc += sizeof(short);
  // skip \0
  while (*loc == '\0') {
    loc++;
  }

  short readDone = 0;
  while (readDone < nFilms) {
    film f;
    int *movieOffset = ((int *)loc + readDone);
    char *movieNameStart = movieStart + *movieOffset;
    f.title = movieNameStart; // it will automatically call constructor and take until \0 only
    char *yearOffset = movieNameStart + strlen(movieNameStart) + 1;
    f.year = 1900 + *yearOffset;
    films.push_back(f);
    readDone++;
  }
  return true; 
}
bool imdb::getCast(const film& movie, vector<string>& players) const { return false; }

imdb::~imdb()
{
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM.. 
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info)
{
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}
