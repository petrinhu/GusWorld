#include "RmlUi_Renderer_SDL.h"
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/Types.h>

// PATCH GUSWORLD (ADR-009): o backend oficial do RmlUi carrega texturas via SDL_image
// (SDL3_image). NAO temos SDL_image no build, e a engine ja decodifica PNG via stb_image
// (a mesma lib do Render2dSdl::load_texture). Por isso LoadTexture abaixo usa stb_image
// (RGBA8 do buffer em memoria) em vez de SDL_image, eliminando a dependencia. O resto do
// backend (GenerateTexture do atlas de glifos do FreeType, geometria, scissor) e intacto.
// stb_image SO o header: a IMPLEMENTACAO (STB_IMAGE_IMPLEMENTATION) ja existe uma vez no
// Render2dSdl - aqui incluimos so as declaracoes (definir de novo daria simbolo dup).
#include "stb_image.h"

#if SDL_MAJOR_VERSION == 2 && !(SDL_VIDEO_RENDER_OGL)
	#error "Only the OpenGL SDL backend is supported."
#endif

static void SetRenderClipRect(SDL_Renderer* renderer, const SDL_Rect* rect)
{
#if SDL_MAJOR_VERSION >= 3
	SDL_SetRenderClipRect(renderer, rect);
#else
	SDL_RenderSetClipRect(renderer, rect);
#endif
}
static void SetRenderViewport(SDL_Renderer* renderer, const SDL_Rect* rect)
{
#if SDL_MAJOR_VERSION >= 3
	SDL_SetRenderViewport(renderer, rect);
#else
	SDL_RenderSetViewport(renderer, rect);
#endif
}

RenderInterface_SDL::RenderInterface_SDL(SDL_Renderer* renderer) : renderer(renderer)
{
	// RmlUi serves vertex colors and textures with premultiplied alpha, set the blend mode accordingly.
	// Equivalent to glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA).
	blend_mode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD, SDL_BLENDFACTOR_ONE,
		SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD);
}

void RenderInterface_SDL::BeginFrame()
{
	SetRenderViewport(renderer, nullptr);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawBlendMode(renderer, blend_mode);
}

void RenderInterface_SDL::EndFrame() {}

Rml::CompiledGeometryHandle RenderInterface_SDL::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices)
{
	GeometryView* data = new GeometryView{vertices, indices};
	return reinterpret_cast<Rml::CompiledGeometryHandle>(data);
}

void RenderInterface_SDL::ReleaseGeometry(Rml::CompiledGeometryHandle geometry)
{
	delete reinterpret_cast<GeometryView*>(geometry);
}

void RenderInterface_SDL::RenderGeometry(Rml::CompiledGeometryHandle handle, Rml::Vector2f translation, Rml::TextureHandle texture)
{
	const GeometryView* geometry = reinterpret_cast<GeometryView*>(handle);
	const Rml::Vertex* vertices = geometry->vertices.data();
	const size_t num_vertices = geometry->vertices.size();
	const int* indices = geometry->indices.data();
	const size_t num_indices = geometry->indices.size();

	Rml::UniquePtr<SDL_Vertex[]> sdl_vertices{new SDL_Vertex[num_vertices]};

	for (size_t i = 0; i < num_vertices; i++)
	{
		sdl_vertices[i].position = {vertices[i].position.x + translation.x, vertices[i].position.y + translation.y};
		sdl_vertices[i].tex_coord = {vertices[i].tex_coord.x, vertices[i].tex_coord.y};

		const auto& color = vertices[i].colour;
#if SDL_MAJOR_VERSION >= 3
		sdl_vertices[i].color = {color.red / 255.f, color.green / 255.f, color.blue / 255.f, color.alpha / 255.f};
#else
		sdl_vertices[i].color = {color.red, color.green, color.blue, color.alpha};
#endif
	}

	SDL_Texture* sdl_texture = (SDL_Texture*)texture;

	SDL_RenderGeometry(renderer, sdl_texture, sdl_vertices.get(), (int)num_vertices, indices, (int)num_indices);
}

void RenderInterface_SDL::EnableScissorRegion(bool enable)
{
	if (enable)
		SetRenderClipRect(renderer, &rect_scissor);
	else
		SetRenderClipRect(renderer, nullptr);

	scissor_region_enabled = enable;
}

void RenderInterface_SDL::SetScissorRegion(Rml::Rectanglei region)
{
	rect_scissor.x = region.Left();
	rect_scissor.y = region.Top();
	rect_scissor.w = region.Width();
	rect_scissor.h = region.Height();

	if (scissor_region_enabled)
		SetRenderClipRect(renderer, &rect_scissor);
}

Rml::TextureHandle RenderInterface_SDL::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source)
{
	Rml::FileInterface* file_interface = Rml::GetFileInterface();
	Rml::FileHandle file_handle = file_interface->Open(source);
	if (!file_handle)
		return {};

	file_interface->Seek(file_handle, 0, SEEK_END);
	size_t buffer_size = file_interface->Tell(file_handle);
	file_interface->Seek(file_handle, 0, SEEK_SET);

	using Rml::byte;
	Rml::UniquePtr<byte[]> buffer(new byte[buffer_size]);
	file_interface->Read(buffer.get(), buffer_size, file_handle);
	file_interface->Close(file_handle);

	// PATCH GUSWORLD: decodifica o PNG/JPG via stb_image (RGBA8) em vez de SDL_image.
	// stb_image le qualquer formato comum direto do buffer em memoria. Sem renderer
	// (headless), nao ha como criar SDL_Texture: devolve handle invalido (o doc degrada).
	if (!renderer)
		return {};

	int w = 0, h = 0, channels = 0;
	stbi_uc* decoded =
		stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(buffer.get()),
							  static_cast<int>(buffer_size), &w, &h, &channels, 4);
	if (!decoded)
		return {};

	texture_dimensions = {w, h};

	// Pre-multiplica o alfa (o RmlUi/SDL compoe com premultiplied alpha; o blend_mode
	// foi configurado para isso no construtor).
	const size_t pixels_byte_size = static_cast<size_t>(w) * static_cast<size_t>(h) * 4;
	for (size_t i = 0; i < pixels_byte_size; i += 4)
	{
		const unsigned int alpha = decoded[i + 3];
		decoded[i + 0] = static_cast<stbi_uc>(static_cast<unsigned int>(decoded[i + 0]) * alpha / 255);
		decoded[i + 1] = static_cast<stbi_uc>(static_cast<unsigned int>(decoded[i + 1]) * alpha / 255);
		decoded[i + 2] = static_cast<stbi_uc>(static_cast<unsigned int>(decoded[i + 2]) * alpha / 255);
	}

	SDL_Surface* surface =
		SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_RGBA32, decoded, w * 4);
	if (!surface)
	{
		stbi_image_free(decoded);
		return {};
	}

	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_DestroySurface(surface);
	stbi_image_free(decoded);

	if (texture)
		SDL_SetTextureBlendMode(texture, blend_mode);

	return (Rml::TextureHandle)texture;
}

Rml::TextureHandle RenderInterface_SDL::GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions)
{
	RMLUI_ASSERT(source.data() && source.size() == size_t(source_dimensions.x * source_dimensions.y * 4));

#if SDL_MAJOR_VERSION >= 3
	auto CreateSurface = [&]() {
		return SDL_CreateSurfaceFrom(source_dimensions.x, source_dimensions.y, SDL_PIXELFORMAT_RGBA32, (void*)source.data(), source_dimensions.x * 4);
	};
	auto DestroySurface = [](SDL_Surface* surface) { SDL_DestroySurface(surface); };
#else
	auto CreateSurface = [&]() {
		return SDL_CreateRGBSurfaceWithFormatFrom((void*)source.data(), source_dimensions.x, source_dimensions.y, 32, source_dimensions.x * 4,
			SDL_PIXELFORMAT_RGBA32);
	};
	auto DestroySurface = [](SDL_Surface* surface) { SDL_FreeSurface(surface); };
#endif

	SDL_Surface* surface = CreateSurface();

	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_SetTextureBlendMode(texture, blend_mode);

	DestroySurface(surface);
	return (Rml::TextureHandle)texture;
}

void RenderInterface_SDL::ReleaseTexture(Rml::TextureHandle texture_handle)
{
	SDL_DestroyTexture((SDL_Texture*)texture_handle);
}
