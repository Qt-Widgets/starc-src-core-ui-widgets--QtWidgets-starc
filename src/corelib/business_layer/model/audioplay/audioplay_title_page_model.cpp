#include "audioplay_title_page_model.h"


namespace BusinessLayer {

class AudioplayTitlePageModel::Implementation
{
public:
    /**
     * @brief Модель информации о проекте
     */
    AudioplayInformationModel* informationModel = nullptr;
};


// ****


AudioplayTitlePageModel::AudioplayTitlePageModel(QObject* _parent)
    : SimpleTextModel(_parent)
    , d(new Implementation)
{
}

AudioplayTitlePageModel::~AudioplayTitlePageModel() = default;

void AudioplayTitlePageModel::setDocumentName(const QString& _name)
{
    Q_UNUSED(_name);
}

void AudioplayTitlePageModel::setInformationModel(AudioplayInformationModel* _model)
{
    d->informationModel = _model;
}

AudioplayInformationModel* AudioplayTitlePageModel::informationModel() const
{
    return d->informationModel;
}

} // namespace BusinessLayer
