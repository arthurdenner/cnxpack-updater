#include "list_translations_tab.hpp"

#include <filesystem>
#include <fstream>
#include <string>

#include "confirm_page.hpp"
#include "list_download_tab_confirmation.hpp"
#include "download.hpp"
#include "utils.hpp"
#include "worker_page.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;

ListTranslationsTab::ListTranslationsTab(const contentType type, const nlohmann::ordered_json& nxlinks) : brls::List(), type(type), nxlinks(nxlinks)
{
    this->setDescription();

    this->createList();
}

void ListTranslationsTab::createList()
{
    ListTranslationsTab::createList(this->type);
}

void ListTranslationsTab::createList(contentType type)
{
    const nlohmann::ordered_json translations = util::getValueFromKey(this->nxlinks, contentTypeNames[(int)type].data());
    if (translations.size()) {
        for (auto it = translations.begin(); it != translations.end(); ++it)
        {
            const std::string title = it.key();
            const std::string folders = (*it)["folders"].get<std::string>();
            const std::string url = (*it)["link"].get<std::string>();


            const std::string text("menus/common/download"_i18n + title);
            listItem = new brls::ListItem(title);
            listItem->setHeight(LISTITEM_HEIGHT);
            listItem->getClickEvent()->subscribe([this, type, text, title, folders, url](brls::View* view) {
                brls::StagedAppletFrame* stagedFrame = new brls::StagedAppletFrame();
                stagedFrame->setTitle(fmt::format("menus/main/getting"_i18n, contentTypeFullNames[(int)type].data()));

                std::vector<std::string> itemFolders;
                itemFolders.clear();

                //splitting folders and inserting into the vector
                size_t pos_start = 0, pos_end, delim_len = 1;
                std::string token;
                while((pos_end = folders.find(",", pos_start)) != std::string::npos) {
                    token = folders.substr(pos_start, pos_end - pos_start);
                    pos_start = pos_end + delim_len;
                    itemFolders.push_back(token);
                }
                itemFolders.push_back(folders.substr(pos_start));
                
                if (util::isTranslationPresent(itemFolders))
				{
                    stagedFrame->addStage(new ListDownloadConfirmationPage(stagedFrame, "menus/main/translation_exists_warning"_i18n));
                    stagedFrame->addStage(new WorkerPage(stagedFrame, "menus/common/deleting"_i18n, [this, type, itemFolders]() { util::doDelete(itemFolders, type); }));
                }
				else
                {
                    stagedFrame->addStage(new ConfirmPage(stagedFrame, text));
                    stagedFrame->addStage(new WorkerPage(stagedFrame, "menus/common/downloading"_i18n, [this, type, url]() { util::downloadArchive(url, type); }));
                    stagedFrame->addStage(new WorkerPage(stagedFrame, "menus/common/extracting"_i18n, [this, type]() { util::extractArchive(type); }));
                }

                stagedFrame->addStage(new ConfirmPage(stagedFrame, "menus/common/all_done"_i18n, true));
                brls::Application::pushView(stagedFrame);
            });
            this->addView(listItem);
        }
    }
    else {
        this->displayNotFound();
    }
}


/*    std::vector<std::pair<std::string, std::string>> links;
    links = download::getLinksFromJson(util::getValueFromKey(this->nxlinks, contentTypeNames[(int)type].data()));

    if (links.size()) {
        for (const auto& link : links) {

            const std::string url = link.second;
            const std::string title = link.first;
            //const std::string title = util::getTranslationName(url);
            //const std::string text("menus/common/download"_i18n + link.first + "menus/common/from"_i18n + url);
            const std::string text("menus/common/download"_i18n + title);
            listItem = new brls::ListItem(title);
            listItem->setHeight(LISTITEM_HEIGHT);
            listItem->getClickEvent()->subscribe([this, type, text, url, title](brls::View* view) {
                brls::StagedAppletFrame* stagedFrame = new brls::StagedAppletFrame();
                stagedFrame->setTitle(fmt::format("menus/main/getting"_i18n, contentTypeFullNames[(int)type].data()));

                std::vector<std::string> itemFolders;
//                std::string itemName;
//                std::string itemLink;
				
//                itemFolders = util::isTranslationPresent(url, itemName, itemLink);
                if (!itemFolders.empty())
                {
                    stagedFrame->addStage(new ListDownloadConfirmationPage(stagedFrame, "menus/main/translation_exists_warning"_i18n));
                    stagedFrame->addStage(new WorkerPage(stagedFrame, "menus/common/deleting"_i18n, [this, type, itemFolders]() { util::doDelete(itemFolders, type); }));
                }
				else
                {
                    stagedFrame->addStage(new ConfirmPage(stagedFrame, text));
                    stagedFrame->addStage(new WorkerPage(stagedFrame, "menus/common/downloading"_i18n, [this, type, url]() { util::downloadArchive(url, type); }));
                    //stagedFrame->addStage(new WorkerPage(stagedFrame, "menus/common/downloading"_i18n, [this, type, itemLink]() { util::downloadArchive(itemLink, type); }));
                    stagedFrame->addStage(new WorkerPage(stagedFrame, "menus/common/extracting"_i18n, [this, type]() { util::extractArchive(type); }));
                }

                stagedFrame->addStage(new ConfirmPage(stagedFrame, "menus/common/all_done"_i18n, true));
                brls::Application::pushView(stagedFrame);
            });
            this->addView(listItem);
        }
    }
    else {
        this->displayNotFound();
    }
}
*/

void ListTranslationsTab::displayNotFound()
{
    brls::Label* notFound = new brls::Label(
        brls::LabelStyle::SMALL,
        "menus/main/links_not_found"_i18n,
        true);
    notFound->setHorizontalAlign(NVG_ALIGN_CENTER);
    this->addView(notFound);
}

void ListTranslationsTab::setDescription()
{
    this->setDescription(this->type);
}

void ListTranslationsTab::setDescription(contentType type)
{
    brls::Label* description = new brls::Label(brls::LabelStyle::DESCRIPTION, "", true);

    switch (type) {
        case contentType::translations: {
            description->setText("menus/main/translations_text"_i18n);
            break;
        }
        default:
            break;
    }

    this->addView(description);
}
