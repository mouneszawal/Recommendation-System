#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define BNUM 8		//book numbers
#define RNUM 20		//readers number
#define NRNUM 5		//new readers numbers
#define LEN 100		

typedef struct K_table {
	int id;
	float sim;
}KTable;

/*
	winBook: will store the final recommended book id.
	target: Stores reader's name the user will enter.
	targetIndex: gets the readers id, in other words, the row it is in.
	users:	Stores the users (readers) names.
	nUsers: Stores the new users names.
	books: Stores the books name.
	rates:	Stores the rating given by x user to y book in a matrix.
	nuRates: Stores the rating given by x new user to y book in a matrix.

*/

/*                                  PROTOTYPES                                       */
void init(char ***books,char ***users,int ***rates,char ***nUsers, int ***nuRates);
void readCsv(char **books,char **users,int **rates,char **nUsers, int **nuRates);
float pearsonCor(int **rates,int rnum,int **nuRates,int nrnum);
float prediction(KTable *table,int **rates,int **nuRates,int nrnum,int bookIndex,int k);
//Just calculates the common read books between two users.
void meanCommon(int **rates,int rnum,int **nuRates,int nrnum,float* ra, float* rb);
KTable * getKtable(KTable *table,int **rates,int **nuRates,int nrnum,int k);
float meanSolo(int **rates,int rnum);
int getRecommendedBookIndex(KTable *table,int **rates,int **nuRates,int nrnum,int k,char **books);
int getIndex(char **nUsers,char *target);
void destroy(char **books,char**users,int **rates,char **nUsers,int **nuRates);

int main(void){
	int i,k,winBook,targetIndex,**rates,**nuRates;
	char **users,**nUsers,**books;
	char *target;
	KTable * simTable = NULL;
	target= malloc(LEN*sizeof(char));
	init(&books,&users,&rates,&nUsers,&nuRates);// allocating memory
	readCsv(books,users,rates,nUsers,nuRates);  // reading the file information

	//read the user name, unti a corresponding user is found
	do{
		printf("Enter the user name: (EX: NU1)\n\t->");
		scanf("%s",target);
		targetIndex = getIndex(nUsers,target);
	}while(targetIndex == 99);

		
	printf("\n_________________________\n\nEnter the similarity table capacity (k) : ");
	scanf("%d", &k);

	simTable = (KTable*)calloc(k+1,sizeof(KTable));
	simTable = getKtable(simTable,rates,nuRates,targetIndex,k);

	printf("\nThe (%d) most similar persons are:\n" , k);
	for(i=0;i<k;i++){
		printf("%d.\t%s\tuser's similarity rate with %s is: %.3f \n", i+1,users[simTable[i].id], nUsers[targetIndex], simTable[i].sim);
	}

	printf("\n\nThe prediction rate of the unread books:\n\n");
	//gets the recommended book index
	winBook=getRecommendedBookIndex(simTable,rates,nuRates,targetIndex,k,books);
	
	printf("\n\nAccording to our statistics..\n%s\tuser should read next:\t%s \n",nUsers[targetIndex],books[winBook] );
	
	destroy(books,users,rates,nUsers,nuRates);//free memory
	return 0;
}

void init(char ***books,char ***users,int ***rates,char ***nUsers, int ***nuRates){
	int i;
	(*rates) =(int**)calloc(RNUM,sizeof(int*));
	for(i=0;i<RNUM;i++){
		(*rates)[i] = (int*)calloc(BNUM,sizeof(int));
	}

	(*nuRates) =(int**)calloc(NRNUM,sizeof(int*));
	for(i=0;i<NRNUM;i++){
		(*nuRates)[i] = (int*)calloc(BNUM,sizeof(int));
	}
	
	(*users) =(char**)calloc(RNUM,sizeof(char*));
	for(i=0;i<RNUM;i++){
		(*users)[i]= (char*)calloc(LEN,sizeof(char));
	}
	
	(*nUsers) =(char**)calloc(NRNUM,sizeof(char*));
	for(i=0;i<NRNUM;i++){
		(*nUsers)[i] = (char*)calloc(LEN,sizeof(char));
	}
	
	(*books) =(char**)calloc(BNUM,sizeof(char*));
	for(i=0;i<BNUM;i++){
		(*books)[i] = (char*)calloc(LEN,sizeof(char));
	}	
	
}

void readCsv(char **books,char **users,int **rates,char **nUsers, int **nuRates){
	int i,j;
	char *line,*val;
	FILE * file = fopen("RecomendationDataSet.csv","r");
	if(file == NULL){
		printf("file not found\n");
	}
	line = (char*)calloc(5*LEN,sizeof(char));
	val = (char*)calloc(LEN,sizeof(char));
	//holding book names;
	fgets(line,500,file);
	val = strtok(line,";");
	i=0;
	
	while(val != NULL){
		val = strtok(NULL,";");
		if(val!=NULL)
			strcpy(books[i],val);
		i++;
	}
	books[BNUM-1][strlen(books[BNUM-1])-1] = '\0';
	
	//holding the readers and the rating matrix
	for(i=0;i<RNUM;i++){
		j=0;
		fgets(line,500,file);
		val = strtok(line,";");
		strcpy(users[i],val);
		while(val !=NULL){
			val = strtok(NULL,";");
			if(val!=NULL){
				rates[i][j]=atoi(val);
				j++;
			}
		}
	}
	//skip one line
	fgets(line,500,file);
	//holding the new users and their rating matrix
	for(i=0;i<NRNUM;i++){
		j=0;
		fgets(line,500,file);
		val = strtok(line,";");
		strcpy(nUsers[i],val);
		while(val !=NULL){
			val = strtok(NULL,";");
			if(val!=NULL){
				nuRates[i][j]=atoi(val);
				j++;
			}
		}
	}
	fclose(file);
}


float pearsonCor(int **rates,int rnum,int **nuRates,int nrnum){
	int i;
	float ra=0,rb=0,sum=0,sdasum=0,sdbsum=0,result=0;
//	meanCommon(rates,rnum,nuRates,nrnum,&ra,&rb);
	ra = meanSolo(rates,rnum);
	rb = meanSolo(nuRates,nrnum);
	for(i=0;i<BNUM;i++){
		if(rates[rnum][i] != 0 && nuRates[nrnum][i] != 0){
			sum    += ( rates[rnum][i] - ra )*( nuRates[nrnum][i] - rb );
			sdasum += ( powf((rates[rnum][i] - ra) ,2) );
			sdbsum += ( powf((nuRates[nrnum][i] - rb) ,2) );
		}
	}
	
	result = sum/(sqrt(sdasum)*sqrt(sdbsum));
	return result;
}

void meanCommon(int **rates,int rnum,int **nuRates,int nrnum,float* ra, float* rb){
	int i,j,asum=0,bsum=0,count=0;
	for(i=0;i<BNUM;i++){
		if(rates[rnum][i] != 0 && nuRates[nrnum][i] != 0){
			asum+= rates[rnum][i];
			bsum+= nuRates[nrnum][i];
			count++;
		}
	}
	
	if(count == 0)
		exit(3);
		
	(*ra) = (float)asum/(float)count;
	(*rb) = (float)bsum/(float)count;
}

float meanSolo(int **rates,int rnum){
	int i,cnt=0,sum=0; 
	float res=0;
	for(i=0;i<BNUM;i++){
		if(rates[rnum][i] != 0 ){
			sum+=rates[rnum][i];
			cnt++;
		}
	}
	if(cnt!=0) //just in case if something went wrong and cnt still zero
		res = (float)sum/(float)cnt;

	return res;	
}

/*
we calculate the similarity between new reader and each readers
then inserts the similarity values with their row number in the
correct cell, when a bigger similarity value comes, first it
slides the table array (k length) in a way that deletes the last
element from the table and insert the new value associated with
its id in the right cell.
time complexity: worst case - O(k).
*/
KTable * getKtable(KTable *table,int **rates,int **nuRates,int tar,int k){
	int i,j,l,flag;
	float similarity;
	for(i=0;i<k;i++){
		table[i].sim = -10;
	}
	
	for(i=0;i<RNUM;i++){
		similarity=pearsonCor(rates,i,nuRates,tar);
		j=0;
		flag=0;
		while(j < k && flag == 0 ){
			if(similarity < table[j].sim){
				j++;
			}else{
				flag = 1;
			}
		}
		for(l=k-1;l>j;l--){
			table[l].sim = table[l-1].sim;
			table[l].id  = table[l-1].id;
		}
		table[j].sim = similarity;
		table[j].id  = i;
	}
	return table;
}

float prediction(KTable *table,int **rates,int **nuRates,int nrnum,int bookIndex,int k){
	int i;
	float predict,ra,rb,pay=0,payda=0;
	for(i=0; i<k ;i++){
		rb=meanSolo(rates,table[i].id);
		pay	  += (table[i].sim) * (rates[table[i].id][bookIndex] - rb);
		payda += table[i].sim;
	}
	ra = meanSolo(nuRates,nrnum);
	if (payda!= 0)
		predict = ra + pay/payda;
	else
		exit(1);

	return predict;
}

int getRecommendedBookIndex(KTable *table,int **rates,int **nuRates,int nrnum,int k,char **books){
	float pred,max=-20.0;
	int i,maxInd;
	
	for(i=0;i<BNUM;i++) {
		if(nuRates[nrnum][i] == 0){
			pred=prediction(table,rates,nuRates,nrnum,i,k);
			if(pred>max){
				max = pred;
				maxInd = i;
			}
			printf("\t%s\t\t%f\n",books[i],pred);
		}
	}
	
	return maxInd;
}

int getIndex(char **nUsers,char *target){
	int i;
	for(i=0;i<NRNUM;i++){
		if( strcmp( nUsers[i], target) == 0 ){
			return i;
		}
	}
	printf("This User is not found in our database.\nPlease try again..\n");
	return 99;
}

void destroy(char **books,char**users,int **rates,char **nUsers,int **nuRates){
	int i;
	for(i=0;i<BNUM;i++){
		free(books[i]);
	}
	free(books);
	
	for(i=0;i<RNUM;i++){
		free(users[i]);
	}
	free(users);
	
	for(i=0;i<BNUM;i++){
		free(rates[i]);
	}
	free(rates);
	
	for(i=0;i<NRNUM;i++){
		free(nUsers[i]);
	}
	free(nUsers);
	
	for(i=0;i<NRNUM;i++){
		free(nuRates[i]);
	}
	free(nuRates);
}
