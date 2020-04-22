/*******************************************
 * Solutions for the CS:APP Performance Lab
 ********************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

/* 
 * Please fill in the following student struct 
 */
student_t student = {
  "Harrison S. Quick",     /* Full name */
  "u1098604@utah.edu",  /* Email address */
};

/***************
 * COMPLEX KERNEL
 ***************/

/******************************************************
 * Your different versions of the complex kernel go here
 ******************************************************/

/* 
 * naive_complex - The naive baseline version of complex 
 */
char naive_complex_descr[] = "naive_complex: Naive baseline implementation";
void naive_complex(int dim, pixel *src, pixel *dest)
{
  int i, j;

  for(i = 0; i < dim; i++)
    for(j = 0; j < dim; j++)
    {

      dest[RIDX(dim - j - 1, dim - i - 1, dim)].red = ((int)src[RIDX(i, j, dim)].red +
						      (int)src[RIDX(i, j, dim)].green +
						      (int)src[RIDX(i, j, dim)].blue) / 3;
      
      dest[RIDX(dim - j - 1, dim - i - 1, dim)].green = ((int)src[RIDX(i, j, dim)].red +
							(int)src[RIDX(i, j, dim)].green +
							(int)src[RIDX(i, j, dim)].blue) / 3;
      
      dest[RIDX(dim - j - 1, dim - i - 1, dim)].blue = ((int)src[RIDX(i, j, dim)].red +
						       (int)src[RIDX(i, j, dim)].green +
						       (int)src[RIDX(i, j, dim)].blue) / 3;

    }
}


/* 
 * complex - Your current working version of complex
 * IMPORTANT: This is the version you will be graded on
 */
char complex_descr[] = "complex: Current working version";
void complex(int dim, pixel *src, pixel *dest)
{
  int i, j, k, l, greyscale;
  pixel destpix; 
  pixel srcpix;

  //Compute flipped matrix transpose similar to q 12 from pre-lecture 10 quiz
  //to make use of locality in cache
      for (i = 0; i < dim; i+= 8)
	for (j = 0; j < dim; j += 8)
	  for (k = i; k < i + 8; k++)
	    {
	      for (l = j; l < j + 8; l++)
		{
		  destpix = dest[RIDX(l, k, dim)];
		  srcpix  = src[RIDX(dim - k - 1, dim - l - 1, dim)];

		  greyscale = ((int)srcpix.red + (int)srcpix.green + (int)srcpix.blue) / 3;

		  destpix.red   = greyscale;
		  destpix.green = greyscale;
		  destpix.blue  = greyscale;

		  dest[RIDX(l, k, dim)] = destpix;
		}
	    }
}

/*********************************************************************
 * register_complex_functions - Register all of your different versions
 *     of the complex kernel with the driver by calling the
 *     add_complex_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_complex_functions() {
  add_complex_function(&complex, complex_descr);
  add_complex_function(&naive_complex, naive_complex_descr);
}


/***************
 * MOTION KERNEL
 **************/

/***************************************************************
 * Various helper functions for the motion kernel
 * You may modify these or add new ones any way you like.
 **************************************************************/


/* 
 * weighted_combo - Returns new pixel value at (i,j) 
 */
static pixel weighted_combo(int dim, int i, int j, pixel *src) 
{
  int ii, jj;
  pixel current_pixel;

  int red, green, blue;
  red = green = blue = 0;

  int num_neighbors = 0;
  for(ii=0; ii < 3; ii++)
    for(jj=0; jj < 3; jj++) 
      if ((i + ii < dim) && (j + jj < dim)) 
      {
	num_neighbors++;
	red += (int) src[RIDX(i+ii,j+jj,dim)].red;
	green += (int) src[RIDX(i+ii,j+jj,dim)].green;
	blue += (int) src[RIDX(i+ii,j+jj,dim)].blue;
      }
  
  current_pixel.red = (unsigned short) (red / num_neighbors);
  current_pixel.green = (unsigned short) (green / num_neighbors);
  current_pixel.blue = (unsigned short) (blue / num_neighbors);
  
  return current_pixel;
}



/* 
 * weighted_combo_3 - Returns new pixel value at (i,j) 
 *
static pixel weighted_combo_3(int dim, int i, int j, pixel *src) 
{
  int ii, jj, iii, jjj;
  pixel current_pixel;

  int red, green, blue;
  red = green = blue = 0;

  iii = i - 1;
  jjj = j - 1;
  for(ii=0; ii < 3; ii++)
    {
      iii++;
      jjj = j;
      for(jj=0; jj < 3; jj++) 
	{
	  jjj++;
	  red += (int) src[RIDX(iii,jjj,dim)].red;
	  green += (int) src[RIDX(iii,jjj,dim)].green;
	  blue += (int) src[RIDX(iii,jjj,dim)].blue;
	}
    } 

  current_pixel.red = (unsigned short) (red / 9);
  current_pixel.green = (unsigned short) (green / 9);
  current_pixel.blue = (unsigned short) (blue / 9);
  
  return current_pixel;
}
*/



/******************************************************
 * Your different versions of the motion kernel go here
 ******************************************************/


/* 
 * weighted_combo_2 - Returns new pixel value at (i,j) 
 
static pixel weighted_combo_2(int dim, int i, int j, pixel *src) 
{
  int ii, jj;
  pixel current_pixel;

  int red, green, blue;
  red = green = blue = 0;

  int diffi = dim - i;
  int diffj = dim - j;
  int num_neighbors = 9;

  if (diffi <= 2)
    {
      if (diffj <= 2)
	num_neighbors = diffi * diffj;
      else
	{
	  num_neighbors = diffi * 3;
	  diffj = 3;
	}
    }
  else if (diffj <= 2)
    {
      num_neighbors = diffj * 3;
      diffi = 3;
    }
  else
      diffi = diffj = 3;

  for(ii=i; ii < i + diffi; ii++)
    {
      for(jj=j; jj < j + diffj; jj++)
	{
	  red += (int) src[RIDX(ii,jj,dim)].red;
	  green += (int) src[RIDX(ii,jj,dim)].green;
	  blue += (int) src[RIDX(ii,jj,dim)].blue;
	}
    }
  current_pixel.red = (unsigned short) (red / num_neighbors);
  current_pixel.green = (unsigned short) (green / num_neighbors);
  current_pixel.blue = (unsigned short) (blue / num_neighbors);
  
  return current_pixel;
}
*/


/*
 * naive_motion - The naive baseline version of motion 
 */
char naive_motion_descr[] = "naive_motion: Naive baseline implementation";
void naive_motion(int dim, pixel *src, pixel *dst) 
{
  int i, j;
    
  for (i = 0; i < dim; i++)
    for (j = 0; j < dim; j++)
      dst[RIDX(i, j, dim)] = weighted_combo(dim, i, j, src);
}


/*
 * motion - Your current working version of motion. 
 * IMPORTANT: This is the version you will be graded on
 */
char motion_descr[] = "motion: Current working version";
void motion(int dim, pixel *src, pixel *dst) 
{
  int i, j;

  for (i = 0; i < (dim - 2); i++)
    for (j = 0; j < (dim - 2); j++)
      {
	dst[RIDX(i, j, dim)].red =    (    src[RIDX(i, j, dim)].red + 
					   src[RIDX(i, j + 1, dim)].red +
					   src[RIDX(i, j + 2, dim)].red +
					   src[RIDX(i + 1, j, dim)].red +
					   src[RIDX(i + 1, j + 1, dim)].red +
					   src[RIDX(i + 1, j + 2, dim)].red +
					   src[RIDX(i + 2, j, dim)].red +
					   src[RIDX(i + 2, j + 1, dim)].red +
					   src[RIDX(i + 2, j + 2, dim)].red
				      ) / 9;

	dst[RIDX(i, j, dim)].green =  (    src[RIDX(i, j, dim)].green + 
					   src[RIDX(i, j + 1, dim)].green +
					   src[RIDX(i, j + 2, dim)].green +
					   src[RIDX(i + 1, j, dim)].green +
					   src[RIDX(i + 1, j + 1, dim)].green +
					   src[RIDX(i + 1, j + 2, dim)].green +
					   src[RIDX(i + 2, j, dim)].green +
					   src[RIDX(i + 2, j + 1, dim)].green +
					   src[RIDX(i + 2, j + 2, dim)].green
				      ) / 9;				       
				       
	dst[RIDX(i, j, dim)].blue =  (     src[RIDX(i, j, dim)].blue + 
					   src[RIDX(i, j + 1, dim)].blue +
					   src[RIDX(i, j + 2, dim)].blue +
					   src[RIDX(i + 1, j, dim)].blue +
					   src[RIDX(i + 1, j + 1, dim)].blue +
					   src[RIDX(i + 1, j + 2, dim)].blue +
					   src[RIDX(i + 2, j, dim)].blue +
					   src[RIDX(i + 2, j + 1, dim)].blue +
					   src[RIDX(i + 2, j + 2, dim)].blue
				      ) / 9;				       
	
      }			       
    
  for (i = 0; i < dim; i++)
    for (j = dim - 2; j < dim; j++)
      {
	dst[RIDX(i, j, dim)] = weighted_combo(dim, i, j, src);
	dst[RIDX(j, i, dim)] = weighted_combo(dim, j, i, src);
      }
}
/*
int i, j, k, kcount, idx, oldidx, tempr, tempg, tempb;
  tempr = tempg = tempb = 0;
  
  pixel temp[dim * dim];
  
  for (i = dim - 1; i >= 0; i--)
    {
      tempr = tempg = tempb = 0;
      for (j = dim - 1; j >= 0; j--)
	{
	  idx = RIDX(i, j, dim);
	  tempr += src[idx].red;
	  tempg += src[idx].green;
	  tempb += src[idx].blue;

	  if (!(j + 2) > dim)
	    {
	      temp[idx].red   = tempr / 3;
	      temp[idx].green = tempg / 3;
	      temp[idx].blue  = tempb / 3;

	      oldidx = RIDX(i, j + 2, dim);
	      tempr -= src[oldidx].red;
	      tempg -= src[oldidx].green;
	      tempb -= src[oldidx].blue;
	    }
	  else
	    {
	      temp[idx].red   = tempr / (dim - j);
	      temp[idx].green = tempg / (dim - j);
	      temp[idx].blue  = tempb / (dim - j);
	    }

	  dst[idx].red   = 
	  dst[idx].green = 
	  dst[idx].blue  = 
	  kcount         = 0;

	  for (k = j; k < (j + 3) && k < dim; k++)
	    {
	      kcount++;
	      oldidx = RIDX(i, k, dim);
	      dst[idx].red   += temp[oldidx].red;
	      dst[idx].green += temp[oldidx].green;
	      dst[idx].blue  += temp[oldidx].blue;
	    }

	  dst[idx].red   /= kcount;
	  dst[idx].green /= kcount;
	  dst[idx].blue  /= kcount;
	}
    }
 */
/*
past motion:

  int i, j;
    
  for (i = 0; i < dim - 2; i++)
    for (j = 0; j < dim - 2; j++)
      dst[RIDX(i, j, dim)] = weighted_combo_3(dim, i, j, src);

  for(; i < dim; i++)
    for(j = dim - 2; j < dim; j++)
      dst[RIDX(i, j, dim)] = weighted_combo(dim, i, j, src);

 */

/********************************************************************* 
 * register_motion_functions - Register all of your different versions
 *     of the motion kernel with the driver by calling the
 *     add_motion_function() for each test function.  When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_motion_functions() {
  add_motion_function(&motion, motion_descr);
  add_motion_function(&naive_motion, naive_motion_descr);
}
