import { Row, Col} from 'antd';

import CreateDefenseProfile from "@/features/defense/defenseprofile/components/CreateDefenseProfile";
import PageTitle from "@/components/common/PageTitle";

export default function ProfileConfig() {
    return (
        <div style={{ padding: '5px 15px' }}>
            <PageTitle title="Create Defense Profile" description="Create your own defense profile for network DDoS protection" />
            <CreateDefenseProfile/>
        </div>
    );
}   