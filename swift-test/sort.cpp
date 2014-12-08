#include <iostream>
using namespace std;

#define size 10

void mergearray(int a[], int first, int mid, int last, int temp[])
{
	int i=first, j=mid+1;
	int m=mid, n=last;
	int k=0;

	while (i<=m && j<=n)
	{
		if(a[i]<=a[j])
			temp[k++]=a[i++];
		else
			temp[k++]=a[j++];
	}

	while(i<=m)
		temp[k++]=a[i++];
	while(j<=n)
		temp[k++]=a[j++];

	for (i=0; i<k; i++)
		a[first+i]=temp[i];
}

void mergesort(int a[], int first, int last, int temp[])
{
	if (first<last)
	{
		int mid=(first+last)/2;
		mergesort(a, first, mid, temp);
		mergesort(a,mid+1, last, temp);
		mergearray(a, first, mid, last, temp);
	}
}
void quick_sort(int s[], int l, int r)
{
	if (l<r)
	{
		int i=l;
		int j=r;
		int x=s[l];
		while(i<j)
		{
			while (i<j && s[j]>=x)
				j--;
			if (i<j)
				s[i++]=s[j];
			while(i<j && s[i]<x)
				i++;
			if (i<j)
				s[j--]=s[i];
		}
		s[i]=x;
		quick_sort(s, l,i-1);
		quick_sort(s,i+1,r);
	}
}
int main()
{
	int a[size]={6,1,9,3,7,8,4,2,5,0};
	int b[size]={16,11,19,13,17,18,14,12,15,10};
	int c[size]={26,21,29,23,27,28,24,22,25,20};
	int d[size]={36,31,39,33,37,38,34,32,35,30};
	int e[size]={46,41,49,43,47,48,44,42,45,40};
//bubble sort;
	for (int i = 0; i<size; ++i)
	{
		for (int j = 0; j<size-1; ++j)
		{
			if (a[j]>a[j+1])
			{
				int temp = a[j];
				a[j]=a[j+1];
				a[j+1]=temp;
			}
		}
	}

	for (int i = 0; i<size; ++i)
	{
		cout<<a[i]<<endl;
	}
//insertion sort
	for (int i=1; i<size; ++i)
	{
		if (b[i-1]>b[i])
		{
			int temp = b[i];
			int j=i;
			while(j>0 && b[j-1]>temp)
			{
				b[j]=b[j-1];
				j--;
			}
			b[j]=temp;
		}
	}
	for (int k=0; k<size; ++k)
	{
		cout<<b[k]<<endl;
	}

//selection sort
	for (int i=0; i<size-1; ++i)
	{
		int index = i;
		for (int j=i+1; j<size; ++j)
		{
			if(c[j]<c[index])
				index = j;
		}
		if (index!=i)
		{
			int temp = c[index];
			c[index]=c[i];
			c[i]=temp;
		}
		
	}
	
	for (int k = 0; k<size; ++k)
	{
		cout<<c[k]<<endl;
	}
//quick sort
	quick_sort(d, 0, size-1);
	for (int i=0; i<size; ++i)
	{
		cout<<d[i]<<endl;
	}
//merge sort
	int temp[size];

	mergesort(e, 0, size-1, temp);

	for (int i=0; i<size; ++i)
	{
		cout<<e[i]<<endl;
	}

	return 0;
}
