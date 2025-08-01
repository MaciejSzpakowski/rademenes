#define VI_VALIDATE
#include "viva_impl.h"
#include "types.h"
#include <cmath>
#define MAX_SPRITE 1000

namespace ph
{
    float gameTime;
    struct mousepos mouse;
}

namespace ph::gl
{
    extern const char diagonalVSSource[];
    const float angle = -3.14159265f * 0.25f;
    const float cos = cosf(angle);
    const float sin = sinf(angle);
    const float sqrtf2 = sqrtf(2);

    vi::system::window wnd;
    vi::gl::renderer g;
    vi::time::timer timer;
    vi::gl::texture fontTexture;
    vi::gl::texture envTextures;
    vi::gl::font font;
    vi::gl::text text;
    vi::gl::text text2;
    float lasttext2update;
    vi::input::mouse mouse;
    vi::input::keyboard keyboard;
    vi::util::rng rng;
    vi::gl::camera worldView;
    int GRID_SPRITE_COUNT;
    ID3D11VertexShader* diagonalVS;

#define LETTERS 400

    vi::gl::sprite sp[MAX_SPRITE];
    vi::gl::sprite grid[2000];
    vi::gl::sprite letters[LETTERS];
    vi::gl::sprite letters2[50];

    char textBuffer[LETTERS];
    char text2Buffer[51];

    void error(const char* str)
    {
        printf(str);
        exit(1);
    }

    void _assert(bool cnd, const char* str)
    {
        if (!cnd) error(str);
    }

    void clearSprites()
    {
        for (uint i = 0; i < MAX_SPRITE; i++)
            sp[i].s1.nodraw = true;
    }

    uint addSprite(float* color, float x, float y, float z, float w, float h)
    {
        for (uint i = 0; i < MAX_SPRITE; i++)
        {
            if (sp[i].s1.nodraw)
            {
                sp[i].s1.nodraw = false;
                sp[i].init(nullptr);
                sp[i].s1.notexture = 1;
                sp[i].s2.col = { color[0],color[1],color[2],1 };
                sp[i].s1.x = x;
                sp[i].s1.y = y;
                sp[i].s1.z = z;
                sp[i].s1.sx = w;
                sp[i].s1.sy = h;
                sp[i].s1.rot = -3.14159265f * 0.25f;
                return i;
            }
        }

        error(__FUNCTION__ ". No more slots.\n");
    }

    uint addSprite(spriteType type, float x, float y, float z, float w, float h)
    {
        const float unitw = 1.0f / 16.0f;
        const float unith = 0.5f;        

        for (uint i = 0; i < 1000; i++)
        {
            if (sp[i].s1.nodraw)
            {
                sp[i].s1.nodraw = false;
                sp[i].init(&envTextures);
                sp[i].s1.x = x * cos - y * sin + sqrtf2 / 2;
                sp[i].s1.y = x * sin + y * cos;
                sp[i].s1.z = z;
                sp[i].s1.sx = w;
                sp[i].s1.sy = h;
                sp[i].s2.uv1 = { (int)type * unitw, 0 , (int)type * unitw + unitw,unith };
                return i;
            }
        }

        error(__FUNCTION__ ". No more slots.\n");
    }

    void updateSprite(uint index, float x, float y)
    {
        massert(index < MAX_SPRITE, "Bad slot");

        sp[index].s1.x = x;
        sp[index].s1.y = y;
    }

    void removeSprite(uint index)
    {
        if (index >= MAX_SPRITE)
            error(__FUNCTION__ ". Bad slot.\n");

        sp[index].s1.nodraw = true;
    }

    void init()
    {
        for (uint i = 0; i < 1000; i++)
        {
            sp[i].s1.nodraw = true;
        }

        // viva
        gameTime = 0;
        worldView = {};
        worldView.scale = 0.1f;
        worldView.aspectRatio = 1920.0f / 1080.0f;
        vi::system::windowInfo winfo = { .height = 1080,.width = 1920,.title = "Pharaoh" };
        wnd.init(&winfo);
        vi::gl::rendererInfo ginfo = {};
        timer.init();
        ginfo.wnd = &wnd;
        ginfo.clearColor[0] = 47 / 255.0f;
        ginfo.clearColor[1] = 79 / 255.0f;
        ginfo.clearColor[2] = 79 / 255.0f;
        ginfo.clearColor[3] = 1;
        g.init(&ginfo);
        diagonalVS = g.createVertexShader(diagonalVSSource);

        // grid
        int GRID_SIZE = 50;
        GRID_SPRITE_COUNT = GRID_SIZE * 2 + 2;
        for (int i=0;i< GRID_SPRITE_COUNT; i++)
        {
            grid[i] = {};
            vi::gl::sprite* s = grid + i;
            s->line.z1 = 0.85f;
            s->line.z2 = 0.85f;
            s->s2.col = { 0.5f, 0.5f, 0.5f, 1 };

            if (i < GRID_SPRITE_COUNT / 2)
            {
                s->line.x1 = i / sqrt(2);
                s->line.y1 = -i / sqrt(2);
                s->line.x2 = i / sqrt(2) + GRID_SPRITE_COUNT / 2.0f / sqrt(2);
                s->line.y2 = -i / sqrt(2) + GRID_SPRITE_COUNT / 2.0f / sqrt(2);
            }
            else
            {
                s->line.x1 = (i- GRID_SPRITE_COUNT / 2) / sqrt(2);
                s->line.y1 = (i - GRID_SPRITE_COUNT / 2) / sqrt(2);
                s->line.x2 = (i - GRID_SPRITE_COUNT / 2) / sqrt(2) + GRID_SPRITE_COUNT / 2.0f / sqrt(2);
                s->line.y2 = (i - GRID_SPRITE_COUNT / 2) / sqrt(2) - GRID_SPRITE_COUNT / 2.0f / sqrt(2);

            }
        }

        // text        
        g.createTextureFromFile(&fontTexture, "font1.png");
        g.createTextureFromFile(&envTextures, "textures.png");
        font.tex = &fontTexture;
        // offset because it broke and i dont know whats happening
        vi::gl::uvSplitInfo usi = { 256,36,0,0,8,12,32,96 };
        g.uvSplit(&usi, font.uv);
        memset(textBuffer, ' ', LETTERS);
        text.init(&font, letters, LETTERS, textBuffer);
        for (uint i = 0; i < LETTERS; i++)
        {
            g.setScreenPos(letters + i, 20, 20);
            letters[i].s2.col = { 1,1,1,1 };
            g.setPixelScale(letters + i, 16, 24);
        }
        text.update();

        // text2
        memset(text2Buffer, 0, 51);
        sprintf(text2Buffer, "");
        text2.init(&font, letters2, 50, text2Buffer);
        for (uint i = 0; i < 50; i++)
        {
            letters2[i].s1.x = -17;
            letters2[i].s1.y = -8.5f;
            letters2[i].s1.z = 0;
            letters2[i].s2.col = { 1,1,1,1 };
            g.setPixelScale(letters2 + i, 16, 24);
        }
        text2.update();

        // mouse
        mouse.init();
        keyboard.init();
    }

    void setText(uint slot, const char* desc)
    {
        textBuffer[0] = 0;
        strcpy(textBuffer, desc);
        text.update();
    }

    void notify(const char* desc)
    {
        lasttext2update = gameTime;
        strcpy(text2Buffer, desc);
        text2.update();
    }

    bool isKeyDown(int key)
    {
        return keyboard.isKeyDown(key);
    }

    bool isKeyPressed(int key)
    {
        return keyboard.isKeyPressed(key);
    }

    bool isKeyReleased(int key)
    {
        return keyboard.isKeyReleased(key);
    }

    void run(VOIDPROC action, VOIDPROC mouseMove, VOIDPROC keyboardstate)
    {
        while (wnd.update())
        {
            timer.update();
            mouse.update(&wnd, &worldView);
            keyboard.update();
            float tick = timer.getTickTimeSec();

            if (keyboard.isKeyDown('W'))
            {
                worldView.y -= tick * 20;
            }
            else if (keyboard.isKeyDown('S'))
            {
                worldView.y += tick * 20;
            }
            if (keyboard.isKeyDown('A'))
            {
                worldView.x -= tick * 20;
            }
            else if (keyboard.isKeyDown('D'))
            {
                worldView.x += tick * 20;
            }

            keyboardstate();

            mouse.getCursorWorldPos(&ph::mouse.fworldx, &ph::mouse.fworldy);
            ph::mouse.worldy = (int)floor(ph::mouse.fworldx * cos - ph::mouse.fworldy * sin);
            ph::mouse.worldx = -(int)floor(ph::mouse.fworldx * sin + ph::mouse.fworldy * cos) - 1;
            mouseMove();

            gameTime = timer.getGameTimeSec();
            action();
            g.beginScene();

            g.updateCamera(&worldView);

            g.setWireframe();
            for (uint i = 0; i < GRID_SPRITE_COUNT; i++)
            {
                g.drawLine(grid + i);
            }

            g.setSolid();
            g.setSpriteVS(diagonalVS);
            for (uint i = 0; i < 1000; i++)
            {
                g.drawSprite(sp + i);
            }
            g.setDefaultSpriteVS();

            // screenview
            g.updateCamera(&g.camera);

            for (uint i = 0; i < LETTERS; i++)
            {
                g.drawSprite(letters + i);
            }

            if (gameTime - lasttext2update > 4 && text2Buffer[0])
            {
                memset(text2Buffer, 0, 50);
                text2.update();
            }

            for (uint i = 0; i < 50; i++)
            {
                g.drawSprite(letters2 + i);
            }

            

            g.endScene();

            Sleep(1);
        }

        g.destroyVertexShader(diagonalVS);
        g.destroy();
        wnd.destroy();
    }
        
    /// <summary>
    /// max size of el is 8 bytes
    /// </summary>
    void shuffleArray(byte* arr, uint len, uint elSize) 
    {
        if (elSize > 8) error(__FUNCTION__". size too big");

        for (uint i = len - 1; i > 0; i--) 
        {
            uint j = rng.rndInt(0, i);
            byte temp[8];
            memcpy(temp, arr + i * elSize, elSize);
            memcpy(arr + i * elSize, arr + j * elSize, elSize);
            memcpy(arr + j * elSize, temp, elSize);
        }
    }

    int rand(int min, int max)
    {
        return rng.rndInt(min, max);
    }

    void centerCamera()
    {
        g.camera.x = 0;
        g.camera.y = 0;
    }

    const char diagonalVSSource[] = R"(
struct sprite
{
    float x,y,z;
    float sx,sy;
    float rot;
    float ox,oy;
    float4 uv;
    float4 color;
};

struct camera
{
	float aspectRatio;
	float x;
	float y;
	float rotation;
	float scale;
};

cbuffer jedziemy: register(b0)
{
	sprite spr;
};

cbuffer poziolo: register(b1)
{
	camera camObj;
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float4 Col : COLOR;
	float2 TexCoord : TEXCOORD;
    uint4 data: COLOR2;
};

static float4 vertices[6] = {
    float4(0, 0, 1.0f, 1.0f),
    float4(0, -1, 1.0f, 0.0f),
    float4(1, 0, 0.0f, 1.0f),
    float4(1, 0, 0.0f, 1.0f),
    float4(0, -1, 1.0f, 0.0f),
	float4(1, -1, 0.0f, 0.0f)
};

static uint2 uv[6] = {
    uint2(0,3),
    uint2(2,3),
    uint2(0,1),
    uint2(0,1),
    uint2(2,3),
	uint2(2,1)
};

VS_OUTPUT main(uint vid : SV_VertexID)
{
	// camera
    // that is correct camera transformation
    // includes aspect ratio adjustment and it FIRST moves camera to postion and then scales
    // so when zooming, it always zooms around the center of the screen
	float4x4 cam = float4x4(
		1/camObj.aspectRatio * camObj.scale, 0, 0, 1/camObj.aspectRatio * camObj.scale * -camObj.x,
		0, camObj.scale, 0, -camObj.scale * -camObj.y,
		0, 0, 1, 0,
		0, 0, 0, 1
	);

	// scale
	float4x4 sca = float4x4(
		spr.sx, 0, 0, 0,
		0, spr.sy, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	);
	// rot
	float4x4 rot = float4x4(
		cos(spr.rot), sin(spr.rot), 0, 0,
		-sin(spr.rot), cos(spr.rot), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	);
	// loc
	float4x4 loc = float4x4(
		1, 0, 0, spr.x,
		0, 1, 0, -spr.y,
		0, 0, 1, 0,
		0, 0, 0, 1 // z from transform is not put over here because it's on the vertex itself
	);
	float4 pos = float4(vertices[vid].x,vertices[vid].y,0,1.0f);

	VS_OUTPUT output;
	output.Pos = mul(mul(mul(mul(cam,rot), loc), sca), pos);
    output.Pos.z = spr.z;
	output.Col = spr.color;
    int u = uv[vid].x;
    int v = uv[vid].y;
    output.TexCoord = float2(spr.uv[u],spr.uv[v]);
    output.data = float4(0,0,0,0);

	return output;
}
)";
}