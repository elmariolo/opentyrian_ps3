/*
 * OpenTyrian: A modern cross-platform port of Tyrian
 * Copyright (C) 2007-2009  The OpenTyrian Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "video.h"

#include "keyboard.h"
#include "opentyr.h"
#include "palette.h"
#include "video_scale.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

bool fullscreen_enabled = false;

SDL_Surface *VGAScreen, *VGAScreenSeg;
SDL_Surface *VGAScreen2;
SDL_Surface *game_screen;

static ScalerFunction scaler_function;

void init_video( void )
{
	if (SDL_WasInit(SDL_INIT_VIDEO))
		return;

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) == -1)
	{
		fprintf(stderr, "error: failed to initialize SDL video: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_WM_SetCaption("OpenTyrian", NULL);

	VGAScreen = VGAScreenSeg = SDL_CreateRGBSurface(SDL_SWSURFACE, vga_width, vga_height, 8, 0, 0, 0, 0);
	VGAScreen2 = SDL_CreateRGBSurface(SDL_SWSURFACE, vga_width, vga_height, 8, 0, 0, 0, 0);
	game_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, vga_width, vga_height, 8, 0, 0, 0, 0);

	SDL_FillRect(VGAScreen, NULL, 0);

	if (!init_scaler(scaler, fullscreen_enabled) &&  // try desired scaler and desired fullscreen state
	    !init_any_scaler(fullscreen_enabled) &&      // try any scaler in desired fullscreen state
	    !init_any_scaler(!fullscreen_enabled))       // try any scaler in other fullscreen state
	{
		fprintf(stderr, "error: failed to initialize any supported video mode\n");
		exit(EXIT_FAILURE);
	}
}

int can_init_scaler( unsigned int new_scaler, bool fullscreen )
{
    // En PS3, siempre aceptamos 32 bits
    return 32;
}

/*
bool init_scaler( unsigned int new_scaler, bool fullscreen )
{
    // Forzamos resolución HD para PS3
    int w = 1280;
    int h = 720;
    
    // bpp 32 es el estándar para HDMI en PS3
    int bpp = 32; 
    int flags = SDL_SWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN;
    
    // Eliminamos la comprobación de can_init_scaler porque ya sabemos que PS3 soporta 720p
    
    SDL_Surface *const surface = SDL_SetVideoMode(w, h, 32, flags);
    
    if (surface == NULL)
    {
        fprintf(stderr, "error: failed to initialize PS3 720p mode: %s\n", SDL_GetError());
        return false;
    }
    
    // Actualizamos las variables para que el resto del motor sepa el tamaño real
    w = surface->w;
    h = surface->h;
    bpp = surface->format->BitsPerPixel;
    
    printf("PS3 Video Initialized: %dx%dx%d (720p High Definition)\n", w, h, bpp);
    
    scaler = new_scaler;
    fullscreen_enabled = true; // En PS3 siempre es true
    
    // Forzamos el uso del escalador de 32 bits
    scaler_function = scalers[scaler].scaler32;
    
	if (scaler_function == NULL)
    {
        // Si el scaler elegido no tiene versión 32-bit, usamos el primero que la tenga
        scaler_function = scalers[0].scaler32; 
    }
    
    input_grab(input_grab_enabled);
    JE_showVGA();
    
    return true;
}*/

/*
bool init_scaler( unsigned int new_scaler, bool fullscreen )
{
    // Forzamos resolución HD para PS3
    int w = 1280;
    int h = 720;
    int bpp = 32; 
    // Añadimos SDL_ANYFORMAT por si el framebuffer de PS3 prefiere otro modo interno
    int flags = SDL_SWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN | SDL_ANYFORMAT;
    
    SDL_Surface *surface = SDL_SetVideoMode(w, h, bpp, flags);
    
    if (surface == NULL)
    {
        fprintf(stderr, "error: failed to initialize PS3 720p mode: %s\n", SDL_GetError());
        return false;
    }
    
    printf("PS3 Video Initialized: %dx%dx%d (Stretch Mode 16:9)\n", surface->w, surface->h, surface->format->BitsPerPixel);
    
    scaler = new_scaler;
    fullscreen_enabled = true; 

       //IMPORTANTE: En PS3, para estirar a pantalla completa 16:9, 
       //no usaremos el scaler_function tradicional. 
       //Lo ponemos en NULL o lo ignoramos en scale_and_flip.
    scaler_function = NULL; 
    
    input_grab(input_grab_enabled);
    JE_showVGA();
    
    return true;
}*/

bool init_scaler( unsigned int new_scaler, bool fullscreen )
{
    // Forzamos el uso de nuestro nn_32 modificado
    scaler = 0; 
    
    int w = 720;
    int h = 480;
    int bpp = 32; 
    int flags = SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN;

    SDL_Surface *surface = SDL_SetVideoMode(w, h, bpp, flags);
    
    if (surface == NULL) {
        // Fallback a software si el hardware falla
        flags = SDL_SWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN;
        surface = SDL_SetVideoMode(w, h, bpp, flags);
    }

    // Asignamos la función modificada
    scaler_function = scalers[scaler].scaler32;
    
    input_grab(input_grab_enabled);
    JE_showVGA();
    
    return true;
}

bool can_init_any_scaler( bool fullscreen )
{
	for (int i = scalers_count - 1; i >= 0; --i)
		if (can_init_scaler(i, fullscreen) != 0)
			return true;
	
	return false;
}

bool init_any_scaler( bool fullscreen )
{
	// attempts all scalers from last to first
	for (int i = scalers_count - 1; i >= 0; --i)
		if (init_scaler(i, fullscreen))
			return true;
	
	return false;
}

void deinit_video( void )
{
	SDL_FreeSurface(VGAScreenSeg);
	SDL_FreeSurface(VGAScreen2);
	SDL_FreeSurface(game_screen);
	
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void JE_clr256( SDL_Surface * screen)
{
	memset(screen->pixels, 0, screen->pitch * screen->h);
}
void JE_showVGA( void ) { scale_and_flip(VGAScreen); }

void scale_and_flip( SDL_Surface *src_surface )
{
	assert(src_surface->format->BitsPerPixel == 8);
	
	SDL_Surface *dst_surface = SDL_GetVideoSurface();
	
	assert(scaler_function != NULL);
	scaler_function(src_surface, dst_surface);
	
	SDL_Flip(dst_surface);
}