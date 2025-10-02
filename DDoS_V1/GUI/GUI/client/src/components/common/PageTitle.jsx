import './common.css'

const PageTitle = ({ title, description, style }) => {
    return (
        <div className="page-title" style={style}>
            <h1>{title}</h1>
            {description && <p>{description}</p>}
        </div>
    );
};

export default PageTitle;