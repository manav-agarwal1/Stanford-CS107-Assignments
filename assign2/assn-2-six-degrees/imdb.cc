using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cassert>
#include "imdb.h"

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";
typedef struct {
  const string *key;
  void *base;
} keyStruct;
typedef struct {
  const film *f;
  void *base;
} filmStruct;

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
static int bsearchCompare (const void* a, const void* b) {
  keyStruct *param = (keyStruct *) a;
  int offset = *(int *) b;

  const char *A = *(const char **) param->key;
  const char *B = ((char *) param->base + offset);
  // printf("%s, %s\n", A, B);
  return strcmp(A, B);
}
bool imdb::getCredits(const string& player, vector<film>& films, short *numCredits, bool onlyNum) const {
  int *start = (int *) actorInfo.fileMap;
  char *movieStart = (char *) movieInfo.fileMap;
  keyStruct *param = new keyStruct;
  param->key = &player;
  param->base = start;

  int *actorOffset = (int *) bsearch (param, (void *) (start+1), *start, sizeof(int), bsearchCompare);
  if (actorOffset == NULL) {
    return false;
  }
  
  char *actorStart = (char *)start + *actorOffset;
  string foundActorName(actorStart);
  assert(player == foundActorName);

  char *loc = (char *) (actorStart + strlen(actorStart));
  // skip padding
  while (*loc == '\0') {
    loc++;
  }
  short nFilms = *(short *)loc;
  if (numCredits != nullptr) {
    *numCredits = nFilms;
    if (onlyNum) {
      return true;
    }
  }
  loc += sizeof(short);
  // skip padding
  while (*loc == '\0') {
    loc++;
  }

  for (int i = 0; i < nFilms; i++) {
    film f;
    int *movieOffset = ((int *)loc + i);
    char *movieNameStart = movieStart + *movieOffset;
    f.title = movieNameStart; // it will automatically call constructor and take until \0 only
    char *yearOffset = movieNameStart + strlen(movieNameStart) + 1;
    f.year = 1900 + *yearOffset;
    films.push_back(f);
  }
  return true; 
}

static int bsearchFilmCompare (const void *a, const void *b) {
  film f;
  filmStruct *A = (filmStruct *) a;
  int *offset = (int *) b;
  char *titleStart = (char *) A->base + *offset;
  f.title = titleStart;
  f.year = 1900 + *(titleStart + strlen(titleStart) + 1);
  if (*(A->f) < f) {
    return -1;
  }
  if (*(A->f) == f) {
    return 0;
  } 
  return 1;
}
bool imdb::getCast(const film& movie, vector<string>& players) const { 
  int *start = (int *) movieInfo.fileMap;
  char *actorStart = (char *) actorInfo.fileMap; 
  filmStruct *param = new filmStruct;
  string movieName = movie.title;
  param->f = &movie;
  param->base = start;
  int *movieOffset = (int *) bsearch (param, (void *) (start+1), *start, sizeof(int), bsearchFilmCompare);
  if (movieOffset == NULL) {
    return false;
  }
  char *movieNameStart = (char *)start + *movieOffset;
  string foundMovieName(movieNameStart);
  assert(movieName == foundMovieName);
  int numActorsOffset = strlen(movieNameStart) + 2; // for \0 and year
  if (numActorsOffset % 2 != 0) {
    numActorsOffset++;
  }
  int numActors = *(short *) ((char *) start + *movieOffset + numActorsOffset);
  int actorsOffset = 2;
  if ((numActorsOffset + 2) % 4 != 0) {
    actorsOffset = 4;
  }
  int *loc = (int *) ((char *) movieFile + *movieOffset + numActorsOffset + actorsOffset);
  for (int i = 0; i < numActors; i++) {
    int actorOffset = *(loc + i);
    string actorName = actorStart + actorOffset;
    players.push_back(actorName);
  }
  return true;
}

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
