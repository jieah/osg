#if defined(_MSC_VER)
	#pragma warning( disable : 4786 )
#endif

#include <osgGA/DriveManipulator>
#include <osgUtil/IntersectVisitor>
#include <osg/Notify>

using namespace osg;
using namespace osgGA;

DriveManipulator::DriveManipulator()
{
    _modelScale = 0.01f;
    _velocity = 0.0f;
    _height = 1.0f;
    _buffer = 1.0f;
    //_speedMode = USE_MOUSE_Y_FOR_SPEED;
    _speedMode = USE_MOUSE_BUTTONS_FOR_SPEED;
}


DriveManipulator::~DriveManipulator()
{
}


void DriveManipulator::setNode(osg::Node* node)
{
    _node = node;
    if (_node.get())
    {
        const osg::BoundingSphere& boundingSphere=_node->getBound();
        _modelScale = boundingSphere._radius;
        _height = sqrtf(_modelScale)*0.03f;
        _buffer = sqrtf(_modelScale)*0.05f;
    }
}


const osg::Node* DriveManipulator::getNode() const
{
    return _node.get();
}


osg::Node* DriveManipulator::getNode()
{
    return _node.get();
}

void DriveManipulator::home(const GUIEventAdapter& ea,GUIActionAdapter& us)
{

    if(_node.get() && _camera.get())
    {

        const osg::BoundingSphere& boundingSphere=_node->getBound();

        osg::Vec3 ep = boundingSphere._center;
        osg::Vec3 bp = ep;
        ep.z() -= _modelScale*0.0001f;
        bp.z() -= _modelScale;

        // check to see if any obstruction in front.
        osgUtil::IntersectVisitor iv;

        bool cameraSet = false;

        osg::ref_ptr<osg::LineSegment> segDown = new osg::LineSegment;
        segDown->set(ep,bp);
        iv.addLineSegment(segDown.get());

        _node->accept(iv);

        if (iv.hits())
        {
            osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(segDown.get());
            if (!hitList.empty())
            {
                //                notify(INFO) << "Hit terrain ok"<< std::endl;
                osg::Vec3 ip = hitList.front().getWorldIntersectPoint();
                osg::Vec3 np = hitList.front().getWorldIntersectNormal();

                osg::Vec3 uv;
                if (np.z()>0.0f) uv = np;
                else uv = -np;

                float lookDistance = _modelScale*0.1f;

                ep = ip;
                ep.z() += _height;
                osg::Vec3 lv = uv^osg::Vec3(1.0f,0.0f,0.0f);
                osg::Vec3 cp = ep+lv*lookDistance;

                _camera->setLookAt(ep,cp,uv);

                cameraSet = true;

            }

        }

        if (!cameraSet)
        {
            bp = ep;
            bp.z() += _modelScale;

            osg::ref_ptr<osg::LineSegment> segUp = new osg::LineSegment;
            segUp->set(ep,bp);
            iv.addLineSegment(segUp.get());

            _node->accept(iv);

            if (iv.hits())
            {
                osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(segUp.get());
                if (!hitList.empty())
                {
                    //                notify(INFO) << "Hit terrain ok"<< std::endl;
                    osg::Vec3 ip = hitList.front().getWorldIntersectPoint();
                    osg::Vec3 np = hitList.front().getWorldIntersectNormal();

                    osg::Vec3 uv;
                    if (np.z()>0.0f) uv = np;
                    else uv = -np;

                    float lookDistance = _modelScale*0.1f;

                    ep = ip;
                    ep.z() += _height;
                    osg::Vec3 lv = uv^osg::Vec3(1.0f,0.0f,0.0f);
                    osg::Vec3 cp = ep+lv*lookDistance;

                    _camera->setLookAt(ep,cp,uv);

                    cameraSet = true;

                }

            }
        }

        if (!cameraSet)
        {
                                 // eye
            _camera->setLookAt(boundingSphere._center+osg::Vec3( 0.0,-2.0f * boundingSphere._radius,0.0f),
                                 // look
                boundingSphere._center,
                                 // up
                osg::Vec3(0.0f,0.0f,1.0f));
        }

    }

    _velocity = 0.0f;

    us.requestRedraw();

    us.requestWarpPointer((ea.getXmin()+ea.getXmax())/2,(ea.getYmin()+ea.getYmax())/2);

    flushMouseEventStack();

    computeLocalDataFromCamera();
}

void DriveManipulator::init(const GUIEventAdapter& ea,GUIActionAdapter& us)
{
    flushMouseEventStack();

    us.requestContinuousUpdate(false);

    _velocity = 0.0f;

    osg::Vec3 ep = _camera->getEyePoint();
    osg::Vec3 sv = _camera->getSideVector();
    osg::Vec3 bp = ep;
    bp.z() -= _modelScale;

    // check to see if any obstruction in front.
    osgUtil::IntersectVisitor iv;

    bool cameraSet = false;

    osg::ref_ptr<osg::LineSegment> segDown = new osg::LineSegment;
    segDown->set(ep,bp);
    iv.addLineSegment(segDown.get());

    _node->accept(iv);

    if (iv.hits())
    {
        osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(segDown.get());
        if (!hitList.empty())
        {
            //                notify(INFO) << "Hit terrain ok"<< std::endl;
            osg::Vec3 ip = hitList.front().getWorldIntersectPoint();
            osg::Vec3 np = hitList.front().getWorldIntersectNormal();

            osg::Vec3 uv;
            if (np.z()>0.0f) uv = np;
            else uv = -np;

            float lookDistance = _modelScale*0.1f;

            ep = ip+uv*_height;
            osg::Vec3 lv = uv^sv;
            osg::Vec3 lp = ep+lv*lookDistance;

            _camera->setLookAt(ep,lp,uv);
            _camera->ensureOrthogonalUpVector();

            cameraSet = true;

        }

    }

    if (!cameraSet)
    {
        bp = ep;
        bp.z() += _modelScale;

        osg::ref_ptr<osg::LineSegment> segUp = new osg::LineSegment;
        segUp->set(ep,bp);
        iv.addLineSegment(segUp.get());

        _node->accept(iv);

        if (iv.hits())
        {
            osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(segUp.get());
            if (!hitList.empty())
            {
                //                notify(INFO) << "Hit terrain ok"<< std::endl;
                osg::Vec3 ip = hitList.front().getWorldIntersectPoint();
                osg::Vec3 np = hitList.front().getWorldIntersectNormal();

                osg::Vec3 uv;
                if (np.z()>0.0f) uv = np;
                else uv = -np;

                float lookDistance = _modelScale*0.1f;

                ep = ip+uv*_height;
                osg::Vec3 lv = uv^sv;
                osg::Vec3 lp = ep+lv*lookDistance;

                _camera->setLookAt(ep,lp,uv);
                _camera->ensureOrthogonalUpVector();

                cameraSet = true;

            }

        }
    }

    if (ea.getEventType()!=GUIEventAdapter::RESIZE)
    {
        us.requestWarpPointer((ea.getXmin()+ea.getXmax())/2,(ea.getYmin()+ea.getYmax())/2);
    }
    
    computeLocalDataFromCamera();
}


bool DriveManipulator::handle(const GUIEventAdapter& ea,GUIActionAdapter& us)
{
    if(!_camera.get()) return false;

    switch(ea.getEventType())
    {
        case(GUIEventAdapter::PUSH):
        {

            addMouseEvent(ea);
            us.requestContinuousUpdate(true);
            if (calcMovement()) us.requestRedraw();
            return true;
        }

        case(GUIEventAdapter::RELEASE):
        {

            addMouseEvent(ea);
            us.requestContinuousUpdate(true);
            if (calcMovement()) us.requestRedraw();
            return true;
        }

        case(GUIEventAdapter::DRAG):
        {

            addMouseEvent(ea);
            us.requestContinuousUpdate(true);
            if (calcMovement()) us.requestRedraw();
            return true;
        }

        case(GUIEventAdapter::MOVE):
        {

            addMouseEvent(ea);
            us.requestContinuousUpdate(true);
            if (calcMovement()) us.requestRedraw();
            return true;
        }

        case(GUIEventAdapter::KEYDOWN):
        {
            if (ea.getKey()==' ')
            {
                flushMouseEventStack();
                home(ea,us);
                us.requestRedraw();
                us.requestContinuousUpdate(false);
                return true;
            }
            else if (ea.getKey()=='q')
            {
                _speedMode = USE_MOUSE_Y_FOR_SPEED;
                return true;
            }
            else if (ea.getKey()=='a')
            {
                _speedMode = USE_MOUSE_BUTTONS_FOR_SPEED;
                return true;
            }
            else if (ea.getKey()=='+')
            {
                _camera->setFusionDistanceRatio(_camera->getFusionDistanceRatio()*1.25f);
                return true;
            }
            else if (ea.getKey()=='-')
            {
                _camera->setFusionDistanceRatio(_camera->getFusionDistanceRatio()/1.25f);
                return true;
            }
            return false;
        }

        case(GUIEventAdapter::FRAME):
        {
            addMouseEvent(ea);
            if (calcMovement()) us.requestRedraw();
            return true;
        }

        case(GUIEventAdapter::RESIZE):
        {
            init(ea,us);
            us.requestRedraw();
            return true;
        }

        default:
            return false;
    }
}

void DriveManipulator::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding("Drive: Space","Reset the viewing position to home");
    usage.addKeyboardMouseBinding("Drive: +","When in stereo, increase the fusion distance");
    usage.addKeyboardMouseBinding("Drive: -","When in stereo, reduse the fusion distance");
    usage.addKeyboardMouseBinding("Drive: q","Use mouse y for controlling speed");
    usage.addKeyboardMouseBinding("Drive: a","Use mouse middle,right mouse buttons for speed");
}


void DriveManipulator::flushMouseEventStack()
{
    _ga_t1 = NULL;
    _ga_t0 = NULL;
}


void DriveManipulator::addMouseEvent(const GUIEventAdapter& ea)
{
    _ga_t1 = _ga_t0;
    _ga_t0 = &ea;
}

void DriveManipulator::computeLocalDataFromCamera()
{
    // maths from gluLookAt/osg::Matrix::makeLookAt
    osg::Vec3 f(_camera->getCenterPoint()-_camera->getEyePoint());
    f.normalize();
    osg::Vec3 s(f^_camera->getUpVector());
    s.normalize();
    osg::Vec3 u(s^f);
    u.normalize();

    osg::Matrix rotation_matrix(s[0],     u[0],     -f[0],     0.0f,
                                s[1],     u[1],     -f[1],     0.0f,
                                s[2],     u[2],     -f[2],     0.0f,
                                0.0f,     0.0f,     0.0f,      1.0f);
                   
    _eye = _camera->getEyePoint();
    _distance = _camera->getLookDistance();
    _rotation.set(rotation_matrix);
    _rotation = _rotation.inverse();
     
}

void DriveManipulator::computeCameraFromLocalData()
{
    osg::Matrix new_rotation;
    new_rotation.makeRotate(_rotation);
    
    osg::Vec3 up = osg::Vec3(0.0f,1.0f,0.0) * new_rotation;
    osg::Vec3 center = (osg::Vec3(0.0f,0.0f,-_distance) * new_rotation) + _eye;

    _camera->setLookAt(_eye,center,up);
}

void DriveManipulator::computeCameraFromLocalData(const osg::Vec3& lv,const osg::Vec3& up)
{
    osg::Vec3 f(lv);
    f.normalize();
    osg::Vec3 s(f^up);
    s.normalize();
    osg::Vec3 u(s^f);
    u.normalize();
    
    osg::Matrix rotation_matrix(s[0],     u[0],     -f[0],     0.0f,
                                s[1],     u[1],     -f[1],     0.0f,
                                s[2],     u[2],     -f[2],     0.0f,
                                0.0f,     0.0f,     0.0f,      1.0f);
                   
    _rotation.set(rotation_matrix);
    _rotation = _rotation.inverse();
    
    computeCameraFromLocalData();
}


bool DriveManipulator::calcMovement()
{
    _camera->setFusionDistanceMode(osg::Camera::PROPORTIONAL_TO_SCREEN_DISTANCE);

    // return if less then two events have been added.
    if (_ga_t0.get()==NULL || _ga_t1.get()==NULL) return false;

    double dt = _ga_t0->time()-_ga_t1->time();

    if (dt<0.0f)
    {
        notify(WARN) << "warning dt = "<<dt<< std::endl;
        dt = 0.0f;
    }

    switch(_speedMode)
    {
        case(USE_MOUSE_Y_FOR_SPEED):
        {
            float my = (_ga_t0->getYmin()+_ga_t0->getYmax())/2.0f;
            float dy = _ga_t0->getY()-my;
            _velocity = -_modelScale*0.0002f*dy;
            break;
        }
        case(USE_MOUSE_BUTTONS_FOR_SPEED):
        {
            unsigned int buttonMask = _ga_t1->getButtonMask();
            if (buttonMask==GUIEventAdapter::LEFT_MOUSE_BUTTON)
            {
                // pan model.

                _velocity += dt*_modelScale*0.02f;

            }
            else if (buttonMask==GUIEventAdapter::MIDDLE_MOUSE_BUTTON ||
                buttonMask==(GUIEventAdapter::LEFT_MOUSE_BUTTON|GUIEventAdapter::RIGHT_MOUSE_BUTTON))
            {

                _velocity = 0.0f;

            }
            else if (buttonMask==GUIEventAdapter::RIGHT_MOUSE_BUTTON)
            {

                _velocity -= dt*_modelScale*0.02f;

            }
            break;
        }
    }

    osg::Matrix rotation_matrix;
    rotation_matrix.makeRotate(_rotation);
    
    osg::Vec3 up = osg::Vec3(0.0f,1.0f,0.0) * rotation_matrix;
    osg::Vec3 lv = osg::Vec3(0.0f,0.0f,-1.0f) * rotation_matrix;

    // rotate the camera.
    float mx = (_ga_t0->getXmin()+_ga_t0->getXmax())/2.0f;

    float dx = _ga_t0->getX()-mx;

    float yaw = -inDegrees(dx*0.1f*dt);

    osg::Quat yaw_rotation;
    yaw_rotation.makeRotate(yaw,up);

    _rotation *= yaw_rotation;
    rotation_matrix.makeRotate(_rotation);
    osg::Vec3 sv = osg::Vec3(1.0f,0.0f,0.0f) * rotation_matrix;

    // movement is big enough the move the eye point along the look vector.
    if (fabsf(_velocity*dt)>1e-8)
    {
        float distanceToMove = _velocity*dt;

        float signedBuffer;
        if (distanceToMove>=0.0f) signedBuffer=_buffer;
        else signedBuffer=-_buffer;

        // check to see if any obstruction in front.
        osgUtil::IntersectVisitor iv;
        osg::ref_ptr<osg::LineSegment> segForward = new osg::LineSegment;
        segForward->set(_eye,_eye+lv*(signedBuffer+distanceToMove));
        iv.addLineSegment(segForward.get());

        _node->accept(iv);

        if (iv.hits())
        {
            osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(segForward.get());
            if (!hitList.empty())
            {
                //                notify(INFO) << "Hit obstruction"<< std::endl;
                osg::Vec3 ip = hitList.front().getWorldIntersectPoint();
                distanceToMove = (ip-_eye).length()-_buffer;
                _velocity = 0.0f;
            }

        }

        // check to see if forward point is correct height above terrain.
        osg::Vec3 fp = _eye+lv*distanceToMove;
        osg::Vec3 lfp = fp-up*_height*5;

        iv.reset();

        osg::ref_ptr<osg::LineSegment> segNormal = new osg::LineSegment;
        segNormal->set(fp,lfp);
        iv.addLineSegment(segNormal.get());

        _node->accept(iv);

        if (iv.hits())
        {
            osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(segNormal.get());
            if (!hitList.empty())
            {
                //                notify(INFO) << "Hit terrain ok"<< std::endl;
                osg::Vec3 ip = hitList.front().getWorldIntersectPoint();
                osg::Vec3 np = hitList.front().getWorldIntersectNormal();

                if (up*np>0.0f) up = np;
                else up = -np;

                _eye = ip+up*_height;
                lv = up^sv;

                computeCameraFromLocalData(lv,up);

                return true;

            }

        }

        // no hit on the terrain found therefore resort to a fall under
        // under the influence of gravity.
        osg::Vec3 dp = lfp;
        dp.z() -= 2*_modelScale;

        iv.reset();

        osg::ref_ptr<osg::LineSegment> segFall = new osg::LineSegment;
        segFall->set(lfp,dp);
        iv.addLineSegment(segFall.get());

        _node->accept(iv);

        if (iv.hits())
        {
            osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(segFall.get());
            if (!hitList.empty())
            {

                notify(INFO) << "Hit terrain on decent ok"<< std::endl;
                osg::Vec3 ip = hitList.front().getWorldIntersectPoint();
                osg::Vec3 np = hitList.front().getWorldIntersectNormal();

                if (up*np>0.0f) up = np;
                else up = -np;

                _eye = ip+up*_height;
                lv = up^sv;

                computeCameraFromLocalData(lv,up);

                return true;
            }
        }

        // no collision with terrain has been found therefore track horizontally.

        lv *= (_velocity*dt);
        
        _eye += lv;


    }

    computeCameraFromLocalData();

    return true;
}
