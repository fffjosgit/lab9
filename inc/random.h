#ifndef JOS_INC_RANDOM_H
#define JOS_INC_RANDOM_H

#define RAND_MAX 0x7FFF

int rand(void);
void srand(unsigned int seed);

#endif /* !JOS_INC_RANDOM_H */