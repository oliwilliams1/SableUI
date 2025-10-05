#include "SableUI/component.h"
#include "SableUI/SableUI.h"

static int s_ctr = 0;
SableUI::BaseComponent::BaseComponent(Colour colour)
{
	s_ctr++;
	m_bgColour = colour;
}

SableUI::BaseComponent::~BaseComponent()
{
	s_ctr--;
	if (rootElement) delete rootElement;
}

void SableUI::BaseComponent::BackendInitialisePanel(Renderer* renderer)
{
	if (rootElement) delete rootElement;

	m_renderer = renderer;

	rootElement = new Element(renderer, ElementType::DIV);
	rootElement->Init(renderer, ElementType::DIV);
	rootElement->setBgColour(m_bgColour);

	SetElementBuilderContext(renderer, rootElement, false);
	Layout();
}

static size_t GetHash(int n, const char* name)
{
	size_t h = 0;

	for (int i = 0; i < strlen(name); i++)
		h = (h << (5 + n)) - h + name[i];

	h ^= (n * 0x9e3779b) ^ (n << 15);

	return h;
}

void SableUI::BaseComponent::BackendInitialiseChild(const char* name, BaseComponent* parent, const ElementInfo& info)
{
	if (rootElement) delete rootElement;

	int n = parent->GetNumChildren();

	m_hash = GetHash(n, name);

	SableUI_Log("Hash for %s_%d is %zu", name, n, m_hash);

	m_renderer = parent->m_renderer;

	StartDiv(info, this);
	Layout();
	EndDiv();
}

SableUI::Element* SableUI::BaseComponent::GetRootElement()
{
	return rootElement;
}

#include <sstream>
#include <iostream>
static inline void hash_combine(std::size_t& seed, std::size_t v) {
	seed ^= v + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
}

static std::size_t ComputeHash(const SableUI::VirtualNode* vnode)
{
	std::size_t h = 1469598103934665603ULL; // FNV offset basis (just a non-zero start)

	// type
	hash_combine(h, std::hash<int>()((int)vnode->type));

	// text/path (if present)
	if (vnode->uniqueTextOrPath.size() != 0)
	{
		std::string s = (std::string)(vnode->uniqueTextOrPath);
		hash_combine(h, std::hash<std::string>()(s));
	}

	// important info fields (treat them as identity/props)
	hash_combine(h, std::hash<int>()((int)vnode->info.wType));
	hash_combine(h, std::hash<int>()((int)vnode->info.hType));

	// combine numeric layout fields
	hash_combine(h, std::hash<int>()(vnode->info.width));
	hash_combine(h, std::hash<int>()(vnode->info.height));
	hash_combine(h, std::hash<int>()(vnode->info.minWidth));
	hash_combine(h, std::hash<int>()(vnode->info.minHeight));
	hash_combine(h, std::hash<int>()(vnode->info.maxWidth));
	hash_combine(h, std::hash<int>()(vnode->info.maxHeight));

	hash_combine(h, std::hash<int>()(vnode->info.marginTop));
	hash_combine(h, std::hash<int>()(vnode->info.marginBottom));
	hash_combine(h, std::hash<int>()(vnode->info.marginLeft));
	hash_combine(h, std::hash<int>()(vnode->info.marginRight));
	hash_combine(h, std::hash<int>()(vnode->info.paddingTop));
	hash_combine(h, std::hash<int>()(vnode->info.paddingBottom));
	hash_combine(h, std::hash<int>()(vnode->info.paddingLeft));
	hash_combine(h, std::hash<int>()(vnode->info.paddingRight));

	hash_combine(h, std::hash<int>()((int)vnode->info.layoutDirection));

	// you can include bg colour if you want it to affect identity:
	hash_combine(h, std::hash<int>()((vnode->info.bgColour.r << 16) ^ (vnode->info.bgColour.g << 8) ^ vnode->info.bgColour.b));

	return h;
}

static std::size_t ComputeHash(const SableUI::Element* elem)
{
	std::size_t h = 1469598103934665603ULL;

	hash_combine(h, std::hash<int>()((int)elem->type));

	if (elem->uniqueTextOrPath.size() != 0)
	{
		std::string s = (std::string)(elem->uniqueTextOrPath);
		hash_combine(h, std::hash<std::string>()(s));
	}

	hash_combine(h, std::hash<int>()((int)elem->wType));
	hash_combine(h, std::hash<int>()((int)elem->hType));

	hash_combine(h, std::hash<int>()(elem->width));
	hash_combine(h, std::hash<int>()(elem->height));
	hash_combine(h, std::hash<int>()(elem->minWidth));
	hash_combine(h, std::hash<int>()(elem->minHeight));
	hash_combine(h, std::hash<int>()(elem->maxWidth));
	hash_combine(h, std::hash<int>()(elem->maxHeight));

	hash_combine(h, std::hash<int>()(elem->marginTop));
	hash_combine(h, std::hash<int>()(elem->marginBottom));
	hash_combine(h, std::hash<int>()(elem->marginLeft));
	hash_combine(h, std::hash<int>()(elem->marginRight));
	hash_combine(h, std::hash<int>()(elem->paddingTop));
	hash_combine(h, std::hash<int>()(elem->paddingBottom));
	hash_combine(h, std::hash<int>()(elem->paddingLeft));
	hash_combine(h, std::hash<int>()(elem->paddingRight));

	hash_combine(h, std::hash<int>()((int)elem->layoutDirection));
	hash_combine(h, std::hash<int>()((elem->bgColour.r << 16) ^ (elem->bgColour.g << 8) ^ elem->bgColour.b));

	return h;
}

#define COLOR_RESET   "\033[0m"
#define COLOR_ELEMENT  "\033[34m"
#define COLOR_VNODE    "\033[32m"
#define COLOR_HASH    "\033[35m"

static std::string PrintTree(SableUI::Element* element, int depth = 0)
{
	std::stringstream ss;
	for (int i = 0; i < depth; i++)
		ss << "  ";

	ss << COLOR_ELEMENT << "El [" << COLOR_HASH << std::to_string(ComputeHash(element)) << COLOR_ELEMENT << "]" << COLOR_RESET << "\n";

	for (SableUI::Child* child : element->children)
	{
		SableUI::Element* childElement = (SableUI::Element*)*child;
		ss << PrintTree(childElement, depth + 1);
	}
	return ss.str();
}

static std::string PrintTree(SableUI::VirtualNode* vnode, int depth = 0)
{
	std::stringstream ss;
	for (int i = 0; i < depth; i++)
		ss << "  ";

	ss << COLOR_VNODE << "Vn [" << COLOR_HASH << std::to_string(ComputeHash(vnode)) << COLOR_VNODE << "]" << COLOR_RESET << "\n";

	for (SableUI::VirtualNode* child : vnode->children)
	{
		ss << PrintTree(child, depth + 1);
	}
	return ss.str();
}

bool SableUI::BaseComponent::Rerender()
{
	Rect oldRect = { rootElement->rect };

	// Generate virtual tree
	SetElementBuilderContext(m_renderer, rootElement, true);
	Layout();
	VirtualNode* virtualRoot = SableUI::GetVirtualRootNode();

	std::cout << "------ START ------\n";
	std::cout << PrintTree(rootElement) << "\n";
	std::cout << PrintTree(virtualRoot) << "\n";
	std::cout << "------- END -------\n";

	rootElement->Reconcile(virtualRoot);

	rootElement->LayoutChildren();
	rootElement->LayoutChildren();

	Rect newRect = rootElement->rect;
	if (oldRect.w != newRect.w || oldRect.h != newRect.h)
		return true;

	needsRerender = false;
	rootElement->Render();

	return false;
}


bool SableUI::BaseComponent::comp_PropagateComponentStateChanges()
{
	bool res = rootElement->el_PropagateComponentStateChanges();

	bool needsFullRerender = false;
	if (needsRerender)
	{
		// Does the re-rendering cause the size of root to be changed?
		// If so, rerender from next significant component
		if (Rerender())
			needsFullRerender = true;
		else
			needsFullRerender = false;
	}


	if (res) needsRerender = true;

	return needsRerender;
}